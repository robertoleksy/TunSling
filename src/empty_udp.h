#ifndef EMPTY_UDP_H
#define EMPTY_UDP_H

#include "iUdp.h"

class empty_udp : public iUdp {
	public:
		size_t send(const unsigned char * data, size_t data_size, const boost::asio::ip::address & adr);
		size_t recv(unsigned char * data, size_t data_size, boost::asio::ip::address & adr_out);
};

#endif // EMPTY_UDP_H
