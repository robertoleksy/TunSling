#ifndef LINUXTUNIMPL_H
#define LINUXTUNIMPL_H

#include "../cjdns-code/NetPlatform.h"
#include <linux/if.h>
#include <linux/if_tun.h>
#include <iostream>
#include <boost/asio/ip/address_v6.hpp>

template <class TStreamDescriptor>
linuxTun<TStreamDescriptor>::linuxTun(std::unique_ptr<TStreamDescriptor> && stream)
:
    m_tun_fd(open("/dev/net/tun", O_RDWR)),
    m_tun_stream(std::move(stream))
{
	std::cout << "Opening TUN\n";
    assert(stream == nullptr);
    if (m_tun_stream == nullptr) throw std::invalid_argument("stream is nullptr");
    if (m_tun_fd == -1) throw std::runtime_error("open /dev/net/tun error");
    m_tun_stream->assign(m_tun_fd);
    assert(m_tun_stream != nullptr);
	std::memset(&m_ifr, 0, sizeof(m_ifr));
	m_ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(m_ifr.ifr_name, "TunSling%d", IFNAMSIZ);
	int errcode_ioctl = ioctl(m_tun_fd, TUNSETIFF, static_cast<void *>(&m_ifr));
	if(errcode_ioctl == -1) throw std::runtime_error("ioctl error");
}

template <class TStreamDescriptor>
void linuxTun<TStreamDescriptor>::set_ip(const boost::asio::ip::address & addr, uint32_t mtu) {
    const int prefix_len = 16;
    std::cout << "Configuring tuntap options: IP address: " << addr << "/" << prefix_len << " MTU=" << mtu << '\n';
    t_syserr err;
    auto binary_address = addr.to_v6().to_bytes();
    err = NetPlatform_addAddress(m_ifr.ifr_name, binary_address.data(), prefix_len, Sockaddr_AF_INET6);
    if (err.my_code != 0) throw std::runtime_error("NetPlatform_addAddress error");
    err = NetPlatform_setMTU(m_ifr.ifr_name, mtu);
    if (err.my_code != 0) throw std::runtime_error("NetPlatform_setMTU error");
    assert(m_tun_stream != nullptr);
    m_tun_stream->release();
    m_tun_stream->assign(m_tun_fd);
    std::cout << "Configuring tuntap options - done\n";
}

template <class TStreamDescriptor>
size_t linuxTun<TStreamDescriptor>::read_from_tun(unsigned char * data, size_t data_size) {
    boost::system::error_code ec;
    return m_tun_stream->read_some(boost::asio::buffer(data, data_size), ec);
}

template <class TStreamDescriptor>
size_t linuxTun<TStreamDescriptor>::send_to_tun(const unsigned char * data, size_t data_size) {
    boost::system::error_code ec;
    return m_tun_stream->write_some(boost::asio::buffer(data, data_size), ec);
}

template <class TStreamDescriptor>
void linuxTun<TStreamDescriptor>::async_read_from_tun(unsigned char * data, size_t data_size, std::function<void(size_t)> handler) {
    m_tun_stream->async_read_some(boost::asio::buffer(data, data_size), 
                                 [handler](boost::system::error_code, size_t bytes_transferred) {
                                     handler(bytes_transferred);
                                 }
    );
}

template <class TStreamDescriptor>
void linuxTun<TStreamDescriptor>::async_write_to_tun(const unsigned char * data, size_t data_size, std::function<void(size_t)> handler) {
    m_tun_stream->async_write_some(boost::asio::buffer(data, data_size), 
                                 [handler](boost::system::error_code, size_t bytes_transferred) {
                                     handler(bytes_transferred);
                                 }
    );
}

template <class TStreamDescriptor>
void linuxTun<TStreamDescriptor>::run() {
	boost::asio::io_service & io_service = m_tun_stream->get_io_service();
	io_service.run();
}

#endif // LINUXTUNIMPL_H
