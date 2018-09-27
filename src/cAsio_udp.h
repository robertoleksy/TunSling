#ifndef CASIO_UDP_H
#define CASIO_UDP_H

#include "iUdp.h"
#include <boost/asio/ip/udp.hpp>


class cAsio_udp : public iUdp
{
    public:
        cAsio_udp(boost::asio::ip::udp::socket && sock);
        /** Default destructor */
        virtual ~cAsio_udp() = default;

        virtual size_t send(const unsigned char * data, size_t data_size, const boost::asio::ip::address & adr);
		virtual size_t recv(unsigned char * data, size_t data_size, boost::asio::ip::address & adr_out);
    private:
        boost::asio::ip::udp::socket m_sock;
        const unsigned short m_port = 9876;
};

#endif // CASIO_UDP_H
