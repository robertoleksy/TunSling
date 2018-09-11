#ifndef LINUXTUNMULTIQUEUE_H
#define LINUXTUNMULTIQUEUE_H

#include "iTun.h"
#include "linuxtun.h"

class linuxTunMultiqueue : public iTun {
	public:
		linuxTunMultiqueue(size_t number_of_queues);
		void set_ip(const boost::asio::ip::address & addr, uint32_t mtu) override;
		size_t read_from_tun(unsigned char * data, size_t data_size) override;
		size_t send_to_tun(const unsigned char * data, size_t data_size) override;
	private:
		boost::asio::io_service m_io_service;
		std::vector<boost::asio::posix::stream_descriptor> m_streams_vector;
		ifreq  m_ifr;
};

#endif // LINUXTUNMULTIQUEUE_H
