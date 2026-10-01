
#include "Config.h"
#include "Server.h"
#include "Settings.h"
#include <csignal>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

Config parseCommandLine(int argc, char* argv[])
{
    std::unordered_map<std::string, std::string> args;
    for (int i { 2 }; i < argc; i += 2)
    {
        std::string key { argv[i - 1] };
        std::string value { argv[i] };
        args[std::move(key)] = std::move(value);
    }

    for (const auto& [key, value] : args)
        std::cout << key << ": " << value << '\n';

    return { .directory { args["--directory"] } };
}

int main(int argc, char* argv[])
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if ((argc - 1) % 2 != 0)
    {
        std::cout << "Usage: ./program --key1 value1 --key2 value2 ...";
        return 1;
    }

    std::signal(SIGPIPE, SIG_IGN);

    // You can use print statements as follows for debugging, they'll be visible
    // when running tests.
    std::cout << "Logs from your program will appear here!\n";

    try
    {
        Server server {
            Settings::port,
            Settings::connectionBacklog,
            parseCommandLine(argc, argv),
            Settings::numThreads,
        };
        server.serve();
    }
    catch (std::runtime_error& ex)
    {
        std::cerr << ex.what() << '\n';
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
    }

    return 0;
}
