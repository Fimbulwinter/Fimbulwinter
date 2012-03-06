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
*                        Sockets Manipulator 	               	    *
* ==================================================================*/


#include "tcp_connection.hpp"
#include "show_message.hpp"

int tcp_connection::tag_counter_;
std::map<int, tcp_connection::pointer> tcp_connection::sessions_;

int tcp_connection::realloc_fifo(unsigned int rfifo_size, unsigned int wfifo_size)
{
	if(max_rdata != rfifo_size && rdata_size < rfifo_size) 
	{
		rdata = (unsigned char *)realloc(rdata, rfifo_size);

		max_rdata = rfifo_size;
	}

	if(max_wdata != wfifo_size && wdata_size < wfifo_size) 
	{
		wdata = (unsigned char *)realloc(wdata, wfifo_size);

		max_wdata = wfifo_size;
	}

	return 0;
}

int tcp_connection::realloc_writefifo(size_t addition)
{
	size_t newsize;

	if(wdata_size + addition  > max_wdata)
	{
		newsize = WFIFO_SIZE;

		while (wdata_size + addition > newsize)
			newsize += WFIFO_SIZE;
	}
	else
	{
		if(max_wdata >= (size_t) 2 * (flags.server ? FIFOSIZE_SERVERLINK : WFIFO_SIZE) && (wdata_size + addition) * 4 < max_wdata)
		{	
			newsize = max_wdata / 2;
		}
		else 
		{
			return 0;
		}
	}

	wdata = (unsigned char *)realloc(wdata, newsize);
	max_wdata = newsize;

	return 0;
}

int tcp_connection::skip(size_t len)
{
	if (rdata_size < rdata_pos + len) 
	{
		len = RFIFOREST(this);
	}

	rdata_pos = rdata_pos + len;

	return 0;
}

void tcp_connection::start()
{
	boost::asio::ip::tcp::no_delay nodelay(true);
	socket_.set_option(nodelay);

	tag_ = ++tag_counter_;
	sessions_[tag_] = this->shared_from_this();

	start_read();
}

void tcp_connection::start_read()
{
	socket_.async_read_some(boost::asio::buffer((char*)(rdata + rdata_size), RFIFOSPACE(this)),
		boost::bind(&tcp_connection::handle_read, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void tcp_connection::handle_read(const boost::system::error_code &error, size_t bytes_transferred)
{
	if (!socket_.is_open() && !flags.eof)
	{
		set_eof();
	}

	if (flags.eof)
		return;

	if (error)
	{
		set_eof();
		do_close();
	}
	else if (bytes_transferred == 0)
	{
		set_eof();
		do_close();
	}
	else
	{
		rdata_size += bytes_transferred;

		if (parse_)
			parse_((pointer)this->shared_from_this());

		RFIFOFLUSH(this);

		start_read();
	}
}

void tcp_connection::send_buffer(size_t len)
{
	size_t newreserve;

	wdata_size += len;

	newreserve = flags.server ? FIFOSIZE_SERVERLINK / 4 : WFIFO_SIZE;

	//realloc_writefifo(newreserve);

	/*boost::asio::async_write(socket_,
		boost::asio::buffer(wdata,
		wdata_size),
		boost::bind(&tcp_connection::handle_write, shared_from_this(),
		boost::asio::placeholders::error));*/
	boost::asio::write(socket_,
		boost::asio::buffer(wdata,
		wdata_size));
	wdata_size = 0;
}

void tcp_connection::handle_write(const boost::system::error_code &error)
{
	if (!error)
	{
		wdata_size = 0;
	}
	else
	{
		set_eof();
		do_close();
	}
}
