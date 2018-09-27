#ifndef IUDP_H
#define IUDP_H

#include <boost/asio/ip/address.hpp>

class iUdp {
    public:
        /** Default destructor */
        virtual ~iUdp() = default;

        virtual size_t send(const unsigned char * data, size_t data_size, const boost::asio::ip::address & adr) = 0;
		/**
		 * @brief recv
		 * @param data
		 * @param data_size
		 * @param adr_out sender ip address
		 * @return numbet of readed bytes
		 */
		virtual size_t recv(unsigned char * data, size_t data_size, boost::asio::ip::address & adr_out) = 0;
};

#endif // IUDP_H
