#include "cSendmmsg_udp.h"

cSendmmsg_udp::cSendmmsg_udp(int socket)
:	m_socket(socket)
{
	
}

cSendmmsg_udp::~cSendmmsg_udp() {
	close(m_socket);
}

size_t cSendmmsg_udp::send(const unsigned char * data, size_t data_size, const boost::asio::ip::address & adr) {
    struct sockaddr_in & my_addr = msgs_q.addr[m_q_len];
    iovec & msgvec = msgs_q.msg[m_q_len];
    msghdr & msg = msgs_q.hdr[m_q_len].msg_hdr;
    std::vector<unsigned char> & msg_data = msgs_q.msg_data[m_q_len];
	msg_data.insert( msg_data.begin(), data, data + data_size );
	
	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_port = htons( m_port ); // short, network byte order
    my_addr.sin_addr.s_addr = *((int*)(adr.to_v4().to_bytes().data()));//inet_addr( "192.168.1.105" );//INADDR_ANY;
    memset( &( my_addr.sin_zero ), '\0', 8 );
	
	msgvec.iov_base = msg_data.data();
	msgvec.iov_len = msg_data.size();
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &my_addr;
	msg.msg_namelen = sizeof(my_addr);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = &msgvec;
	msg.msg_iovlen = 1;

	m_q_len++;
	if( m_q_len == m_q_max_len ) {
		size_t bytes_sended = 0;
        int ret = sendmmsg(m_socket, msgs_q.hdr.data(), m_q_len, 0);
        for(unsigned int i = 0; i < m_q_len; i++ ) {
            bytes_sended += msgs_q.hdr[i].msg_len;
            msgs_q.msg_data[i].clear();
        }
        m_q_len = 0;
        return bytes_sended;
	}
	return 0;
}

size_t cSendmmsg_udp::recv(unsigned char * data, size_t data_size, boost::asio::ip::address & adr_out) {
	throw std::runtime_error("Not implemented");
	return 0;
/*	struct sockaddr_in remote_addr, my_addr;
	socklen_t addrlen = sizeof(my_addr);

	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_port = htons( m_port ); // short, network byte order
	my_addr.sin_addr.s_addr = *((int*)(adr.to_v4().to_bytes().data()));//INADDR_ANY;
	memset( &( my_addr.sin_zero ), '\0', 8 );
	if (bind(m_socket,(struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) // TODO
		throw std::runtime_error("bind error");

	ssize_t readed_bytes= recvfrom(m_socket, data, data_size, 0, (struct sockaddr *) &remote_addr, &addrlen);
	adr_out = boost::asio::ip::address_v4(ntohl(remote_addr.sin_addr.s_addr));

	if (readed_bytes == -1) return 0;
	return readed_bytes;*/
}
