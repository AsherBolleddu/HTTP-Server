#include "Config.hpp"
#include "Server.hpp"
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>
#include <unordered_map>

Config parseCommandLine(int argc, char* argv[])
{
    std::unordered_map<std::string_view, std::string_view> args;
    for (int i { 2 }; i <= argc - 1; i += 2)
        args.insert({ argv[i - 1], argv[i] });

    return { .directory { args["--directory"] } };
}

int main(int argc, char* argv[])
{
    std::signal(SIGPIPE, SIG_IGN);
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if ((argc - 1) % 2 != 0)
    {
        std::cout << "Usage: ./your_program.sh --key1 value1 --key2 value2";
        return EXIT_FAILURE;
    }

    try
    {
        Server server { parseCommandLine(argc, argv) };
        server.serve();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
    }

    return 0;
}
