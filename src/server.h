#pragma once
#include <array>
#include <set>
#include <mutex>
#include <iostream>
#include <utility>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "async.h"

namespace ba = boost::asio;

//boost::asio::io_service io_service;

const std::size_t buff_size = 1024;

std::mutex session_mutex;

/*
class participant
{
public:
    virtual ~participant() {}
};
*/

//typedef std::shared_ptr<session> participant_ptr;

class session
   // : public participant,
    : public std::enable_shared_from_this<session>
{
public:
    session(ba::ip::tcp::socket socket, std::set<std::shared_ptr<session>>& cl, std::size_t bs)
        : socket_(std::move(socket)),
        cl_(cl), bl_size_(bs), id_(nullptr)
    {
    }

    void start_session()
    {
        cl_.insert(shared_from_this());
        id_ = async::connect(bl_size_);
        buff_ = std::make_shared<std::array<char, buff_size>>();
        std::cout << "start session, id = " << (reinterpret_cast<std::size_t>(id_)) << "\n";
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        std::cout << "read session, id = " << (reinterpret_cast<std::size_t>(id_)) << "\n";
        ba::async_read(socket_,
            boost::asio::buffer(buff_->data(), buff_size),
            [this, self](boost::system::error_code ec, std::size_t length)
            {
               std::cout << "async_read, lenght = " << length << "\n";
                if (!ec)
                {
                 std::cout << "receive, id = " << (reinterpret_cast<std::size_t>(id_)) << "\n";
                    async::receive(id_, buff_->data(), length);
                    do_read();
                }
               else
                {
                 std::cout << "disconnect, id = " << (reinterpret_cast<std::size_t>(id_)) << "\n";
                    async::disconnect(id_);
                    cl_.erase(shared_from_this());
                }
            });
    }

    ba::ip::tcp::socket socket_;
    std::set<std::shared_ptr<session>>& cl_;
    std::size_t bl_size_;
    async::handle_t id_;
    std::shared_ptr<std::array<char, buff_size>> buff_;
};


class server
{
public:
    server(ba::io_service& io_service, const ba::ip::tcp::endpoint& endpoint, std::size_t size)
     //   : service_(io_service),
    //    endpoint_(ba::ip::tcp::v4(), port),
        : acceptor_(io_service, endpoint),
        socket_(io_service), 
        bulk_size_(size)
    {
            std::cout << "конструктор сервера\n";
        do_accept();
    }

private:
    void do_accept()
    {
        std::cout << "do_accept\n";
       acceptor_.async_accept(socket_,
            [this](boost::system::error_code ec)
            {
                if (!ec)
                {
                    std::cout << "make shared session\n";
                    std::make_shared<session>(std::move(socket_), clients_, bulk_size_)->start_session();
                }

                do_accept();
            });
    }

 //   ba::io_service& service_;
 //   ba::ip::tcp::endpoint endpoint_;
    ba::ip::tcp::acceptor acceptor_;
    //ba::ip::tcp::endpoint endpoint_(ba::ip::tcp::v4(), std::atoi(argv[1]));
    ba::ip::tcp::socket socket_;
    std::set<std::shared_ptr<session>> clients_;
    std::size_t bulk_size_;
};


  /*  
    boost::asio::io_service service;
boost::asio::ip::tcp::acceptor acceptor(service);
const size_t buffer_size = 1024;

void on_read(std::shared_ptr<boost::asio::ip::tcp::socket> sock,
             std::shared_ptr<std::array<char, buffer_size>> buff,
             async::handle_t handle,
             const boost::system::error_code &e,
             size_t len) {
    if (!e) {
        async::receive(handle, buff->data(), len);
        sock->async_read_some(boost::asio::buffer(buff->data(), buffer_size),
                              boost::bind(on_read, sock, buff, handle, _1, _2));
    } else {
        async::disconnect(handle);
        sock->close();
    }
}


void on_accept(std::shared_ptr<boost::asio::ip::tcp::socket> sock, size_t bulk, const boost::system::error_code &e) {
    auto socketPtr = std::make_shared<boost::asio::ip::tcp::socket>(service);
    acceptor.async_accept(*socketPtr, boost::bind(on_accept, socketPtr, bulk, _1));
    if (!e) {
        auto handle = async::connect(bulk);
        auto buff = std::make_shared<std::array<char, buffer_size>>();
        sock->async_read_some(boost::asio::buffer(buff->data(), buffer_size),
                              boost::bind(on_read, sock, buff, handle, _1, _2));
    }
}

void server(unsigned short port, size_t bulk) {
    auto ip = boost::asio::ip::address_v4::any();
    boost::asio::ip::tcp::endpoint ep(ip, port);
    acceptor = boost::asio::ip::tcp::acceptor(service, ep);
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(service);
    acceptor.async_accept(*socket, boost::bind(on_accept, socket, bulk, _1));
    service.run();
}
*/
