/*==================================================================*
*     ___ _           _           _          _       _				*
*    / __(_)_ __ ___ | |__  _   _| |_      _(_)_ __ | |_ ___ _ __	*
*   / _\ | | '_ ` _ \| '_ \| | | | \ \ /\ / / | '_ \| __/ _ \ '__|	*
*  / /   | | | | | | | |_) | |_| | |\ V  V /| | | | | ||  __/ |		*
*  \/    |_|_| |_| |_|_.__/ \__,_|_| \_/\_/ |_|_| |_|\__\___|_|		*
*																	*
* ------------------------------------------------------------------*
*							 Emulator   			                *
* ------------------------------------------------------------------*
*                     Licenced under GNU GPL v3                     *
* ----------------------------------------------------------------- *
*                          Socket Modules 	               	        *
* ==================================================================*/

#pragma once

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "tcp_connection.hpp"

using namespace boost::asio::ip;

class tcp_server
{
private:
	tcp::acceptor acceptor_;
	tcp_connection::parse default_parser_;

public:
	tcp_server(boost::asio::io_service &io_service, address &addr, unsigned short port)
		: acceptor_(io_service, tcp::endpoint(addr, port))
	{
		default_parser_ = 0;

		start_accept();
	}

	void start_accept()
	{
		tcp_connection::pointer new_connection = tcp_connection::create(acceptor_.get_io_service());

		acceptor_.async_accept(new_connection->socket(),
			boost::bind(&tcp_server::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
	}

	void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
	{
		if (!error)
		{
			new_connection->set_parser(default_parser_);
			new_connection->start();
		}

		start_accept();
	}

	void set_default_parser(tcp_connection::parse p)
	{
		default_parser_ = p;
	}
};
