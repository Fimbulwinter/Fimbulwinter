#pragma once

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <malloc.h>
#include <stdint.h>

using namespace boost::asio::ip;

#define FIFOSIZE_SERVERLINK (256 * 1024)

#define RFIFOHEAD(cl)
#define WFIFOHEAD(cl, size) if(cl->wdata_size + (size) > cl->max_wdata) realloc_writefifo(fd, size);
#define RFIFOP(cl,pos) (cl->rdata + cl->rdata_pos + (pos))
#define WFIFOP(cl,pos) (cl->wdata + cl->wdata_size + (pos))

#define RFIFOB(cl,pos) (*(unsigned char*)RFIFOP(fd,pos))
#define WFIFOB(cl,pos) (*(unsigned char*)WFIFOP(fd,pos))
#define RFIFOW(cl,pos) (*(unsigned short*)RFIFOP(fd,pos))
#define WFIFOW(cl,pos) (*(unsigned short*)WFIFOP(fd,pos))
#define RFIFOL(cl,pos) (*(unsigned int*)RFIFOP(fd,pos))
#define WFIFOL(cl,pos) (*(unsigned int*)WFIFOP(fd,pos))
#define RFIFOQ(cl,pos) (*(unsigned long long*)RFIFOP(fd,pos))
#define WFIFOQ(cl,pos) (*(unsigned long long*)WFIFOP(fd,pos))
#define RFIFOSPACE(cl) (cl->max_rdata - cl->rdata_size)
#define WFIFOSPACE(cl) (cl->max_wdata - cl->wdata_size)

#define RFIFOREST(cl)  (cl->flags.eof ? 0 : cl->rdata_size - cl->rdata_pos)
#define RFIFOFLUSH(cl) \
	if(cl->rdata_size == cl->rdata_pos) { \
		cl->rdata_size = cl->rdata_pos = 0; \
	} else { \
		cl->rdata_size -= cl->rdata_pos; \
		memmove(cl->rdata, cl->rdata + cl->rdata_pos, cl->rdata_size); \
		cl->rdata_pos = 0; \
	}

#define RBUFP(p,pos) (((unsigned char*)(p)) + (pos))
#define RBUFB(p,pos) (*(unsigned char*)RBUFP((p),(pos)))
#define RBUFW(p,pos) (*(unsigned short*)RBUFP((p),(pos)))
#define RBUFL(p,pos) (*(unsigned int*)RBUFP((p),(pos)))
#define RBUFQ(p,pos) (*(unsigned long long*)RBUFP((p),(pos)))

#define WBUFP(p,pos) (((unsigned char*)(p)) + (pos))
#define WBUFB(p,pos) (*(unsigned char*)WBUFP((p),(pos)))
#define WBUFW(p,pos) (*(unsigned short*)WBUFP((p),(pos)))
#define WBUFL(p,pos) (*(unsigned int*)WBUFP((p),(pos)))
#define WBUFQ(p,pos) (*(unsigned long long*)WBUFP((p),(pos)))

#define TOB(n) ((unsigned char)((n)&UCHAR_MAX))
#define TOW(n) ((unsigned short)((n)&USHORT_MAX))
#define TOL(n) ((unsigned int)((n)&UINT_MAX))
#define TOQ(n) ((unsigned long long)((n)&ULLONG_MAX))

#define RFIFO_SIZE (2 * 1024)
#define WFIFO_SIZE (16 * 1024)
#define WFIFO_MAX (1 * 1024 * 1024)

class tcp_connection
	: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;
	typedef void (*parse)(pointer cl);

	static pointer create(boost::asio::io_service &io_service)
	{
		return pointer(new tcp_connection(io_service));
	}

	tcp::socket &socket()
	{
		return socket_;
	}

	void do_close()
	{
		socket_.close();
	}

	bool is_eof() 
	{
		return flags.eof == 1;
	}

	void set_eof()
	{
		flags.eof = 1;

		if (parse_)
			parse_(this->shared_from_this());
	}

	void *get_data()
	{
		return data_;
	}

	void set_data(char *data)
	{
		data_ = data;
	}

	void set_parser(parse p)
	{
		parse_ = p;
	}

	void start();
	void send_buffer(size_t len);
	int skip(size_t len);

	int realloc_writefifo(size_t addition);
	int realloc_fifo(int fd, unsigned int rfifo_size, unsigned int wfifo_size);

	unsigned char *rdata, *wdata;
	size_t max_rdata, max_wdata;
	size_t rdata_size, wdata_size;
	size_t rdata_pos;

	struct 
	{
		unsigned char eof : 1;
		unsigned char server : 1;
	} flags;

	~tcp_connection()
	{
		free(rdata);
		free(wdata);
	}

private:
	tcp_connection(boost::asio::io_service &io_service)
		: socket_(io_service)
	{
		flags.eof = 0;
		flags.server = 0;

		data_ = NULL;
		parse_ = NULL;

		max_rdata  = RFIFO_SIZE;
		max_wdata  = WFIFO_SIZE;

		rdata = (unsigned char *)malloc(max_rdata);
		wdata = (unsigned char *)malloc(max_wdata);

		rdata_size = rdata_pos = 0;
		wdata_size = 0;
	}

	void start_read();
	void handle_write(const boost::system::error_code& error);
	void handle_read(const boost::system::error_code &error, size_t bytes_transferred);

	tcp::socket socket_;
	void *data_;
	parse parse_;
};
