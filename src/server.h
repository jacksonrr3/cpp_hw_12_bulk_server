#pragma once
#include<array>
#include<set>
#include<boost/asio.hpp>
#include"async.h"

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
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        ba::async_read(socket_,
            boost::asio::buffer(buff_->data(), buff_size),
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
    server(ba::io_service& io_service, int port, std::size_t size)
        : service_(io_service),
    //    endpoint_(ba::ip::tcp::v4(), port),
        acceptor_(io_service, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)),
      //  socket_(io_service), 
        bulk_size_(size)
    {
        do_accept();
    }

private:
    void do_accept()
    {
       auto socket_ = std::make_shared<ba::ip::tcp::socket>(service_);
        acceptor_.async_accept(*socket_,
            [this, socket_](boost::system::error_code ec)
            {
                if (!ec)
                {
                    std::make_shared<session>(std::move(*socket_), clients_, bulk_size_)->start_session();
                }

                do_accept();
            });
    }

    ba::io_service& service_;
 //   ba::ip::tcp::endpoint endpoint_;
    ba::ip::tcp::acceptor acceptor_;
    //ba::ip::tcp::endpoint endpoint_(ba::ip::tcp::v4(), std::atoi(argv[1]));
    //ba::ip::tcp::socket socket_;
    std::set<std::shared_ptr<session>> clients_;
    std::size_t bulk_size_;
};