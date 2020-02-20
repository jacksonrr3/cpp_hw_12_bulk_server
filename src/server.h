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

const std::size_t buff_size = 1024;

std::mutex session_mutex;

class Session
    : public std::enable_shared_from_this<Session>
{
public:
    Session( std::shared_ptr<ba::ip::tcp::socket> socket, std::set<std::shared_ptr<Session>>& cl, std::size_t bs)
        : socket_(socket),
        cl_(cl), bl_size_(bs), id_(nullptr)
    {
    }
    
    ~Session(){
          async::disconnect(id_);
    }
    
    void start_session()
    {
    session_mutex.lock();
        cl_.insert(shared_from_this());
    session_mutex.unlock();    
        id_ = async::connect(bl_size_);
        buff_ = std::make_shared<std::array<char, buff_size>>();
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        socket_->async_read_some(boost::asio::buffer(buff_->data(), buff_size),
        [this, self](boost::system::error_code ec, std::size_t length)
            {
                  if (!ec)
                {
                    async::receive(id_, buff_->data(), length);
                    do_read();
                }
               else
                {
                     async::disconnect(id_);
                 session_mutex.lock();   
                    cl_.erase(shared_from_this());
                 session_mutex.unlock();   
                }
            });
    }

    std::shared_ptr<ba::ip::tcp::socket> socket_;
    std::set<std::shared_ptr<Session>>& cl_;
    std::size_t bl_size_;
    async::handle_t id_;
    std::shared_ptr<std::array<char, buff_size>> buff_;
};


class Server
{
public:
    Server(ba::io_service& io_service, const ba::ip::tcp::endpoint& endpoint, std::size_t size)
        : service_(io_service),
        acceptor_(io_service, endpoint),
        bulk_size_(size)
    {
        socket_ = std::make_shared<ba::ip::tcp::socket>(service_);    
        do_accept();
    }
    
private:
    void do_accept()
    {
       acceptor_.async_accept(*socket_,
            [this](boost::system::error_code ec)
            {
                if (!ec)
                {
                    std::make_shared<Session>(socket_, clients_, bulk_size_)->start_session();
                }
                socket_ = std::make_shared<ba::ip::tcp::socket>(service_);    
                do_accept();
            });
    }

    ba::io_service& service_;
    ba::ip::tcp::acceptor acceptor_;
    std::shared_ptr<ba::ip::tcp::socket> socket_;
    std::set<std::shared_ptr<Session>> clients_;
    std::size_t bulk_size_;
};
