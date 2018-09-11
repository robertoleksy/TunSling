#include "cNode_factory.h"
#include <array>
#include "cSecretbox_wrapper.h"
#include "cX_salsa_20_wrapper.h"
#include "ccryptoempty.h"
#include "linuxtun.h"
#include "clinuxtunweld.h"
#include "cAsio_udp.h"
#include "cSendmmsg_udp.h"
#include "empty_udp.h"
#include <thread>
#include "linuxTunMultiqueue.h"

std::unique_ptr<node> cNode_factory::create_node( const boost::program_options::variables_map & vm ) {
    std::unique_ptr<node> ret = std::make_unique<node>();
	
	ret->m_dst_addr = boost::asio::ip::address::from_string(vm["address"].as<std::string>());
	
	std::string strTun = vm["tun"].as<std::string>();
	ret->m_io_service = std::make_unique<boost::asio::io_service>();
	assert(ret->m_io_service != nullptr);
	auto stream_descriptor = std::make_unique<boost::asio::posix::stream_descriptor>(*(ret->m_io_service));
	std::string tunAddr = vm["tunAddr"].as<std::string>() + ":1111:2222:3333:4444:5555:6666:7777";
	
	if( vm.count("tunMultiQueueSync") ) {
		int th = vm["threads"].as<int>();
		if( th <= 0 )
			throw std::invalid_argument( "Wrong threads number" );
		ret->m_tun = std::make_unique<linuxTunMultiqueue>(th);
	} else if( vm.count("tunMultiThread") ) {
		std::cout << "creating async tun\n";
		// Create async tun
		ret->m_tun_async = std::make_unique<linuxTun<>>(std::move(stream_descriptor));
		ret->m_tun_async->set_ip(boost::asio::ip::address::from_string(tunAddr), vm["tunMtu"].as<int>()); // MTU
	} else {
		//Create tun (sync)
		std::cout << "creating blocking tun\n";
		if( strTun == "LinuxNormal" ) {
			ret->m_tun = std::make_unique<linuxTun<>>(std::move(stream_descriptor));
		} else if ( strTun == "LinuxWeld" ) {
			ret->m_tun = std::make_unique<cLinuxTunWeld>(std::move(stream_descriptor));
		} else {
			throw std::runtime_error( "Unknown tun version" );
		}
		ret->m_tun->set_ip(boost::asio::ip::address::from_string(tunAddr), vm["tunMtu"].as<int>()); // MTU
	}
	assert(stream_descriptor == nullptr);

	//Create crypto
	std::string strCrypto = vm["crypto"].as<std::string>();
	if ( strCrypto == "Secretbox" ) {
		ret->m_crypto = std::make_unique<cSecretbox_wrapper>();
	} else if ( strCrypto == "X_salsa_20" ) {
		ret->m_crypto = std::make_unique<cX_salsa_20_wrapper>();
	} else if ( strCrypto == "Empty" ) {
		ret->m_crypto = std::make_unique<cCryptoEmpty>();
	} else {
		throw std::runtime_error( "Unknown crypto version" );
	}
	
	//Create UDP
	std::string strUdp = vm["UDP"].as<std::string>();
	if( strUdp == "Asio" ) {
		boost::asio::ip::udp::socket socket(*(ret->m_io_service));
		socket.open(boost::asio::ip::udp::v4());
		ret->m_udp = std::make_unique<cAsio_udp>(std::move(socket));
	} else if( strUdp == "Sendmmsg" ) {
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		ret->m_udp = std::make_unique<cSendmmsg_udp>(sockfd);
	} else if( strUdp == "Empty" ) {
		ret->m_udp = std::make_unique<empty_udp>();
	} else
		throw std::runtime_error( "Unknown UDP version" );

	//Create thread pool
	int threads = vm["threads"].as<int>();
	if( threads == 0 ) {
		
	} else if( threads > 0 ) {
		ret->m_thread_pool = std::make_unique<ThreadPool>(threads);
	} else
		throw std::runtime_error( "Wrong threads number" );
	
	return ret;
}
