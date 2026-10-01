[![progress-banner](https://backend.codecrafters.io/progress/http-server/fbc814db-b93d-478f-a8a8-637a5335dc28)](https://app.codecrafters.io/users/AsherBolleddu?r=2qF)

# HTTP Server in C++23

A multithreaded HTTP/1.1 server written from scratch on POSIX sockets, with no networking or HTTP libraries. Built as a solution to the CodeCrafters ["Build Your Own HTTP Server"](https://app.codecrafters.io/courses/http-server/overview) challenge.

## Features

- **Thread pool** with a fixed set of workers pulling accepted connections off a mutex- and condition-variable-guarded queue
- **Persistent connections** (keep-alive by default, `Connection: close` honored and echoed back)
- **gzip compression** of response bodies via zlib, negotiated through `Accept-Encoding`
- **File serving and upload** from a directory chosen at startup
- **RAII socket wrapper**: move-only, closes its file descriptor on destruction, so no code path leaks a connection
- **Dual-stack listener**: a single IPv6 socket that also accepts IPv4 clients
- **Request validation**: malformed requests get `400`, bodies over 1 MiB get `413`, and file paths containing `..` or a leading `/` are rejected

## Quick start

### Requirements

- Linux or macOS
- CMake 3.13 or newer
- A compiler with C++23 library support (the code uses `std::out_ptr` and `std::string_view::contains`)
- [vcpkg](https://github.com/microsoft/vcpkg), with `VCPKG_ROOT` set (it supplies zlib)

### Build and run

```sh
./your_program.sh --directory /tmp/files
```

The script configures CMake, builds, and starts the server on port `4221`. To do the steps by hand:

```sh
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
cmake --build ./build
./build/http-server --directory /tmp/files
```

`--directory` is the root for the `/files/` endpoints. If it is omitted, file paths resolve relative to the working directory.

### Try it

```sh
curl -i http://localhost:4221/
curl -i http://localhost:4221/echo/hello
curl -i --compressed http://localhost:4221/echo/hello
curl -i http://localhost:4221/user-agent
curl -i --data "some content" http://localhost:4221/files/notes.txt
curl -i http://localhost:4221/files/notes.txt
```

## Endpoints

| Method | Path            | Response                                                                                        |
| ------ | --------------- | ----------------------------------------------------------------------------------------------- |
| any    | `/`             | `200 OK`, empty body                                                                            |
| any    | `/echo/{text}`  | `200 OK`, `{text}` as `text/plain`; gzip-compressed if the client sends `Accept-Encoding: gzip` |
| any    | `/user-agent`   | `200 OK`, the request's `User-Agent` value as `text/plain`; `404` if the header is missing      |
| `GET`  | `/files/{name}` | `200 OK`, file contents as `application/octet-stream`; `404` if it is not a regular file        |
| `POST` | `/files/{name}` | `201 Created` after writing the request body to the file                                        |

Anything else returns `404 Not Found`.

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
2. `Server::serve` accepts connections on the main thread and moves each `Socket` into the `ThreadPool`.
3. A worker runs `Server::handleClient`, which loops for the life of the connection: read until the end of the headers, parse, read the rest of the body according to `Content-Length`, route, and send the response.
4. `HTTP::route` matches the request target and calls a function in `Handler`, which returns an `HTTP::Response` value. Handlers never touch the socket, so they can be tested without a network.

### Project layout

| File                      | Responsibility                                                      |
| ------------------------- | ------------------------------------------------------------------- |
| `src/main.cpp`            | Argument parsing, `SIGPIPE` handling, server startup                |
| `src/Server.{h,cpp}`      | Listener setup, accept loop, per-connection request loop            |
| `src/ThreadPool.{h,cpp}`  | Worker threads and the connection queue                             |
| `src/Socket.{h,cpp}`      | RAII file descriptor wrapper with `recvAll`, `recvExact`, `sendAll` |
| `src/HTTP.{h,cpp}`        | Request and response types, parsing, serialization, routing         |
| `src/Handler.{h,cpp}`     | Endpoint logic, encoding negotiation, gzip compression              |
| `src/Settings.h`          | Compile-time settings                                               |
| `src/Config.h`            | Runtime settings from the command line                              |
| `src/FailedError.{h,cpp}` | `errno` to message formatting                                       |

### Design notes

- **Errors as values in the request path.** Parsers return `std::optional` and the server turns an empty result into a `400`. Exceptions are reserved for startup failures (`socket`, `bind`, `listen`), where the process cannot continue.
- **Short reads and writes are handled.** `sendAll` and `recvExact` loop until the full byte count has moved or the peer goes away.
- **Case-insensitive headers.** Header names are lowercased at parse time, so lookups are a plain map `find`.
- **`SIGPIPE` is ignored**, so a client that disconnects mid-response produces a failed `send` rather than killing the process.
- **Clean shutdown of the pool.** The destructor sets a stop flag under the lock, wakes every worker, and joins them; workers drain the queue before exiting.

## Configuration

Compile-time settings live in `src/Settings.h`:

| Setting             | Default | Meaning                           |
| ------------------- | ------- | --------------------------------- |
| `port`              | `4221`  | Listening port                    |
| `connectionBacklog` | `5`     | `listen()` backlog                |
| `numThreads`        | `8`     | Worker threads in the pool        |
| `maxBodySize`       | 1 MiB   | Largest accepted `Content-Length` |
| `validSchemes`      | `gzip`  | Supported content encodings       |

## Limitations

This is a learning project, not a production server. Known gaps:

- No `Transfer-Encoding: chunked`; bodies require `Content-Length`
- No read or idle timeouts, so a silent client holds a worker thread
- No cap on header size
- Pipelined requests are not supported: bytes read past the first request's headers are treated as its body
- `/`, `/echo/` and `/user-agent` do not check the request method
- No TLS, and only HTTP/1.1
- No graceful shutdown on `SIGINT` or `SIGTERM`
