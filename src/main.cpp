#include "Server.hpp"
#include <csignal>
#include <exception>
#include <iostream>

int main()
{
    std::signal(SIGINT, SIG_IGN);
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible
    // when running tests.
    std::cout << "Logs from your program will appear here!\n";

    try
    {
        Server server {};
        server.serve();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
    }

    return 0;
}
