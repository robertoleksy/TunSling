#include "empty_udp.h"

size_t empty_udp::send(const unsigned char *, size_t data_size, const boost::asio::ip::address &) {
	return data_size;
}

size_t empty_udp::recv(unsigned char *, size_t data_size, boost::asio::ip::address &) {
	return data_size;
}
