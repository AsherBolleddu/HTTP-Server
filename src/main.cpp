
#include "Server.h"
#include "Settings.h"
#include <csignal>
#include <exception>
#include <iostream>
#include <stdexcept>

int main()
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::signal(SIGPIPE, SIG_IGN);

    // You can use print statements as follows for debugging, they'll be visible
    // when running tests.
    std::cout << "Logs from your program will appear here!\n";

    try
    {
        Server server { Settings::port, Settings::connectionBacklog, Settings::numThreads };
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
