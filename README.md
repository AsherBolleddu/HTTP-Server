[![progress-banner](https://backend.codecrafters.io/progress/http-server/4d0088e8-31f2-48d4-bf24-df5a9391bd68)](https://app.codecrafters.io/users/AsherBolleddu?r=2qF)

# Multithreaded HTTP Server

A multithreaded HTTP/1.1 server written from scratch on POSIX sockets, with no networking or HTTP libraries. Built as a solution to the CodeCrafters ["Build Your Own HTTP Server"](https://app.codecrafters.io/courses/http-server/overview) challenge.

## Features

- **Thread pool** with a fixed set of workers pulling accepted connections off a mutex- and condition-variable-guarded queue
- **Persistent connections** (keep-alive by default, `Connection: close` honored and echoed back)
- **gzip compression** of response bodies via zlib, negotiated through `Accept-Encoding`
- **File serving and upload** from a directory chosen at startup
- **Table-driven routing**: a path-prefix → method → handler map, so an unknown path is `404` and a known path with the wrong method is `405`
- **Errors as values**: parsers and handlers return `std::expected<T, HTTP::Status>`, and the server turns an error into the matching status response
- **RAII socket wrapper**: move-only, closes its file descriptor on destruction, so no code path leaks a connection
- **Dual-stack listener**: a single IPv6 socket that also accepts IPv4 clients
- **Request validation**: malformed requests get `400`, bodies over 1 MiB get `413`, and file paths containing `..` or a leading `/` are rejected

## Performance

Measured with [`wrk`](https://github.com/wg/wrk) over loopback on an Apple Silicon laptop, Release build, 8 keep-alive connections against the 8-thread pool:

| Endpoint      | Requests/sec | p50   | p99   |
| ------------- | ------------ | ----- | ----- |
| `/`           | 131k         | 59 µs | 84 µs |
| `/echo/hello` | 125k         | 63 µs | 89 µs |

```sh
wrk -t4 -c8 -d30s --latency http://localhost:4221/echo/hello
```

Loopback numbers measure the server's per-request overhead, not real network behavior.

## Quick start

### Requirements

- Linux or macOS
- CMake 3.21 or newer and Ninja
- A compiler with C++23 library support (tested with Apple clang 21 on macOS arm64; the code uses `std::expected`, `std::out_ptr` and `std::string_view::contains`)
- [vcpkg](https://github.com/microsoft/vcpkg), with `VCPKG_ROOT` set (it supplies zlib)

### Build and run

```sh
./run.sh --directory /tmp/files            # Debug build
./run.sh --release --directory /tmp/files  # Release build
```

The script configures CMake from `CMakePresets.json`, builds, and starts the server on port `4221`. To do the steps by hand:

```sh
cmake --preset default
cmake --build --preset release
./build/Release/http-server --directory /tmp/files
```

`--directory` is the root for the `/files/` endpoints. If it is omitted, those endpoints return `404`.

### Try it

```sh
curl -i http://localhost:4221/
curl -i http://localhost:4221/echo/hello
curl -i --compressed http://localhost:4221/echo/hello
curl -i http://localhost:4221/user-agent
curl -i --data "some content" http://localhost:4221/files/notes.txt
curl -i http://localhost:4221/files/notes.txt
curl -i -X POST http://localhost:4221/echo/hello   # 405
```

## Endpoints

| Method | Path            | Response                                                                                        |
| ------ | --------------- | ----------------------------------------------------------------------------------------------- |
| `GET`  | `/`             | `200 OK`, empty body                                                                            |
| `GET`  | `/echo/{text}`  | `200 OK`, `{text}` as `text/plain`; gzip-compressed if the client sends `Accept-Encoding: gzip` |
| `GET`  | `/user-agent`   | `200 OK`, the request's `User-Agent` value as `text/plain`; `400` if the header is missing      |
| `GET`  | `/files/{name}` | `200 OK`, file contents as `application/octet-stream`; `404` if it is not a regular file        |
| `POST` | `/files/{name}` | `201 Created` after writing the request body to the file                                        |

An unknown path returns `404 Not Found`; a known path with an unsupported method returns `405 Method Not Allowed`. Every response carries a `Content-Length`, so bodiless responses are correctly framed on keep-alive connections.

## How it works

```
accept loop (main thread)
        │  Socket (moved)
        ▼
  task queue  ──►  worker threads (8)
                         │
                         ▼
        recv headers ► parse ► read body ► route ► handler ► serialize ► send
                         ▲                                              │
                         └──────────── keep-alive loop ◄────────────────┘
```

1. `Server::makeListener` resolves the address with `getaddrinfo`, creates the socket, sets `SO_REUSEADDR`, clears `IPV6_V6ONLY`, then binds and listens.
2. `Server::serve` accepts connections on the main thread, sets a 5-second receive timeout on each, and moves the `Socket` into the `ThreadPool`.
3. A worker runs `handleClient`, which loops for the life of the connection: read until the end of the headers, parse, read the rest of the body according to `Content-Length`, route, and send the response.
4. `Router::execute` splits the target into a prefix and a remainder, looks up the prefix and method in the dispatch table, and calls the handler with the remainder, the headers, the body and the runtime config. Handlers never touch the socket, so they can be tested without a network.

### Project layout

| File                       | Responsibility                                                      |
| -------------------------- | ------------------------------------------------------------------- |
| `src/main.cpp`             | Argument parsing, `SIGPIPE` handling, server startup                |
| `src/Server.{hpp,cpp}`     | Listener setup, accept loop, per-connection request loop            |
| `src/ThreadPool.{hpp,cpp}` | Worker threads and the connection queue                             |
| `src/Socket.{hpp,cpp}`     | RAII file descriptor wrapper with `recvAll`, `recvExact`, `sendAll` |
| `src/HTTP.{hpp,cpp}`       | Request and response types, parsing, serialization                  |
| `src/Router.{hpp,cpp}`     | Dispatch table, endpoint handlers, encoding negotiation, gzip       |
| `src/Settings.hpp`         | Compile-time settings                                               |
| `src/Config.hpp`           | Runtime settings from the command line                              |

### Design notes

- **Errors as values in the request path.** Parsers and handlers return `std::expected<T, HTTP::Status>`; the error type *is* the response status, so the server's error handling is a single `sendResponse(client, { .status = result.error() })`. Exceptions are reserved for startup failures (`socket`, `bind`, `listen`), where the process cannot continue.
- **Zero-copy request parsing.** `RequestLine` fields and header values are `std::string_view`s into the receive buffer; only header names are copied (lowercased) and only the body is owned.
- **`Content-Length` is enforced at serialization.** `HTTP::serialize` adds the header from the body size whenever a handler did not set it, so no handler can produce an unframed response.
- **Short reads and writes are handled.** `sendAll` and `recvExact` loop until the full byte count has moved or the peer goes away, and a short body read closes the connection rather than routing a truncated request.
- **Case-insensitive headers.** Header names are lowercased at parse time, so lookups are a plain map `find`.
- **`SIGPIPE` is ignored**, so a client that disconnects mid-response produces a failed `send` rather than killing the process.
- **Clean shutdown of the pool.** The destructor sets a stop flag under the lock, wakes every worker, and joins them; workers drain the queue before exiting.

## Configuration

Compile-time settings live in `src/Settings.hpp`:

| Setting             | Default | Meaning                           |
| ------------------- | ------- | --------------------------------- |
| `port`              | `4221`  | Listening port                    |
| `connectionBacklog` | `5`     | `listen()` backlog                |
| `nthreads`          | `8`     | Worker threads in the pool        |
| `maxBodySize`       | 1 MiB   | Largest accepted `Content-Length` |
| `validSchemes`      | `gzip`  | Supported content encodings       |

The build enables `-Wall -Wextra -Weffc++ -Wconversion -Wsign-conversion -pedantic-errors -Werror`.
