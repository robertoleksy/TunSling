#ifndef LINUXTUNMULTIQUEUE_H
#define LINUXTUNMULTIQUEUE_H

#include "iTun.h"
#include "linuxtun.h"
#include <map>
#include <thread>

class linuxTunMultiqueue : public iTun {
	public:
		linuxTunMultiqueue(size_t number_of_queues);
		void set_ip(const boost::asio::ip::address & addr, uint32_t mtu) override;
		/**
		 * @brief read_from_tun should be called multiple times in multiple threads
		 * number of threads must be <= number_of_queues used in c-tor
		 * otherwise UB (data race)
		 */
		size_t read_from_tun(unsigned char * data, size_t data_size) override;
		size_t send_to_tun(const unsigned char * data, size_t data_size) override;
	private:
		boost::asio::io_service m_io_service;
		std::vector<int> m_fd_vector;
		ifreq  m_ifr;
		std::atomic<size_t> m_fd_index;
};

#endif // LINUXTUNMULTIQUEUE_H
