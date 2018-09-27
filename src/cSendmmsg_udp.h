#ifndef CSENDMMSG_UDP_H
#define CSENDMMSG_UDP_H

#include <array>
#include <vector>
#include "iUdp.h"

namespace detail {
    template <std::size_t N>
    struct msgs {
        const std::size_t size = N;
        std::array<mmsghdr, N> hdr;
        std::array<sockaddr_in, N> addr;
        std::array<iovec, N> msg;
        std::array<std::vector<unsigned char>, N> msg_data;
	};
}

class cSendmmsg_udp : public iUdp
{
public:
	cSendmmsg_udp(int socket);
	/** Default destructor */
	virtual ~cSendmmsg_udp();

	virtual size_t send(const unsigned char * data, size_t data_size, const boost::asio::ip::address & adr);
	virtual size_t recv(unsigned char * data, size_t data_size, boost::asio::ip::address & adr_out);
private:
	int m_socket;
    constexpr static unsigned int m_q_max_len = 10;
	unsigned int m_q_len = 0;
    detail::msgs<m_q_max_len> msgs_q;
	const unsigned short m_port = 9876;
};

#endif // CSENDMMSG_UDP_H
