#include <iostream>
#include "server.h"


int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: chat_server <port> <bulk_size>\n";
            return 1;
        }
       /*
        ba::io_service io_service;
        server bulk_server(io_service, std::atoi(argv[1]), std::atoi(argv[2]));
        io_service.run();
        */
            std::istringstream ss(argv[1]);
    unsigned int port;
    ss >> port;
    if (port < 1 || port > 65535) {
        std::cerr << "Порт должен быть числом от 1 до 65535" << std::endl;
        return EXIT_FAILURE;
    }
    const size_t max_block_size = 1000000000;
    size_t block_size;
    ss = std::istringstream(argv[2]);
    ss >> block_size;
    if (block_size == 0 || block_size > max_block_size) {
        std::cerr << "Размер блока должен быть целым, положительным числом, не более "
                  << max_block_size << std::endl;
        return EXIT_FAILURE;
    }
        server(port, block_size);
    }
    
    catch (std::exception & e)
        
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
