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
       
        ba::io_service io_service;
        ba::ip::tcp::endpoint endpoint(ba::ip::tcp::v4(), std::atoi(argv[1]));

        server bulk_server(io_service, endpoint, std::atoi(argv[2]));
        io_service.run();
        
   
      //  server(std::atoi(argv[1]), std::atoi(argv[2]));
    
    }
    catch (std::exception & e)
        
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
