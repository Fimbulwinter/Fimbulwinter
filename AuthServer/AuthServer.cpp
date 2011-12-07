#include <iostream>
#include <tcp_server.hpp>
#include <soci/soci.h>
#include <soci/soci-mysql.h>

void parse_login(tcp_connection::pointer cl)
{
	size_t len = RFIFOREST(cl);

	memmove(WFIFOP(cl, 0), RFIFOP(cl, 0), len);

	cl->send_buffer(len);
	cl->skip(len);
}

int main(int argc, char *argv[])
{


	try
	{
		boost::asio::io_service io_service;
		tcp_server server(io_service, (address)address_v4::any(), 6900);
		server.set_default_parser(parse_login);

		io_service.run();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	getchar();

	return 0;
}
