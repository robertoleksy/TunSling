#include "linuxTunMultiqueue.h"


linuxTunMultiqueue::linuxTunMultiqueue(size_t number_of_queues)
{
	for (size_t i = 0; i < number_of_queues; i++)
		m_streams_vector.emplace_back(m_io_service);
	std::memset(&m_ifr, 0, sizeof(m_ifr));
	m_ifr.ifr_flags = IFF_TUN | IFF_NO_PI | IFF_MULTI_QUEUE;
	strncpy(m_ifr.ifr_name, "TunSling%d", IFNAMSIZ);
	for (size_t i = 0; i < number_of_queues; i++) {
		int fd;
		if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
			throw std::runtime_error("Tun open error"); // TODO use std::system_error
		m_streams_vector.at(i).assign(fd);
		int err = ioctl(fd, TUNSETIFF, reinterpret_cast<void *>(&m_ifr));
		if (err) {
			close(fd);
			throw std::runtime_error("ioctl error: " + std::to_string(err));
		}
	}
}

void linuxTunMultiqueue::set_ip(const boost::asio::ip::address & addr, uint32_t mtu) {
    const int prefix_len = 16;
    std::cout << "Configuring tuntap options: IP address: " << addr << "/" << prefix_len << " MTU=" << mtu << '\n';
    t_syserr err;
    auto binary_address = addr.to_v6().to_bytes();
    err = NetPlatform_addAddress(m_ifr.ifr_name, binary_address.data(), prefix_len, Sockaddr_AF_INET6);
    if (err.my_code != 0) throw std::runtime_error("NetPlatform_addAddress error");
    err = NetPlatform_setMTU(m_ifr.ifr_name, mtu);
    if (err.my_code != 0) throw std::runtime_error("NetPlatform_setMTU error");
    for (auto & posix_stream : m_streams_vector) {
        int fd = posix_stream.native_handle();
        posix_stream.release();
        posix_stream.assign(fd);
    }
    std::cout << "Configuring tuntap options - done\n";
}

size_t linuxTunMultiqueue::read_from_tun(unsigned char * data, size_t data_size) {
    size_t stream_index = m_read_stream_number.fetch_add(1) % m_streams_vector.size();
    boost::system::error_code ec;
    return m_streams_vector.at(stream_index).read_some(boost::asio::buffer(data, data_size), ec);
}

size_t linuxTunMultiqueue::send_to_tun(const unsigned char * data, size_t data_size) {
    throw std::runtime_error("not implemented");
    return 0;
}
