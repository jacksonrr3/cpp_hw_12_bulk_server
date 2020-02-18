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
   
        server(std::atoi(argv[1]), std::atoi(argv[2]));
    
    }
    catch (std::exception & e)
        
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
