
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

    return { .directory { args["--directory"] } };
}

int main(int argc, char* argv[])
{
    if ((argc - 1) % 2 != 0)
    {
        std::cout << "Usage: ./program --key1 value1 --key2 value2 ...";
        return 1;
    }

    std::signal(SIGPIPE, SIG_IGN);

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
