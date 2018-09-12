#include "linuxTunMultiqueue.h"


linuxTunMultiqueue::linuxTunMultiqueue(size_t number_of_queues)
:
	m_fd_index(0)
{
	std::memset(&m_ifr, 0, sizeof(m_ifr));
	m_ifr.ifr_flags = IFF_TUN | IFF_NO_PI | IFF_MULTI_QUEUE;
	strncpy(m_ifr.ifr_name, "TunSling%d", IFNAMSIZ);
	for (size_t i = 0; i < number_of_queues; i++) {
		int fd = open("/dev/net/tun", O_RDWR);
		if (fd < 0)
			throw std::runtime_error("Tun open error"); // TODO use std::system_error
		m_fd_vector.emplace_back(fd);
		std::cout << "assign fd " << fd << '\n';
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
    std::cout << "Configuring tuntap options - done\n";
}

size_t linuxTunMultiqueue::read_from_tun(unsigned char * data, size_t data_size) {
    static thread_local int fd = m_fd_vector.at(m_fd_index.fetch_add(1));
    ssize_t ret = read(fd, data, data_size);
    if (ret <= 0) return 0;
    return ret;
}

size_t linuxTunMultiqueue::send_to_tun(const unsigned char * data, size_t data_size) {
    throw std::runtime_error("not implemented");
    return 0;
}
