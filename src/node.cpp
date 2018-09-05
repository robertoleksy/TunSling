#include "node.h"

#include <array>
#include "cbuffermanager.h"
#include "cSecretbox_wrapper.h"
#include "linuxtun.h"
#include "cAsio_udp.h"
#include "cSendmmsg_udp.h"
#include <thread>

node::node()
	:
	  m_buffer_manager(100, 1500*20) // for possible weld data
{
	m_crypto_key.fill(0b10101010); // TODO: use real generation
}

void node::run() {
	assert(m_tun != nullptr);
	assert(m_io_service != nullptr);
    cBufferManager bufferManager(100, 1500 * 20); // for possible weld data
	while (true) {
//        std::vector<unsigned char> buffer(1500 * 20); // for possible weld data
        cBuffer & buffer = bufferManager.get_free_buffer_or_wait();
		size_t tun_read_size = m_tun->read_from_tun(buffer.data(), buffer.size());
		if( m_thread_pool ) {
            m_thread_pool->addJob([=,&buffer]() mutable {
					size_t encypted_message_size =
						m_crypto->encrypt(
							  buffer.data(), tun_read_size,
							  m_crypto_key.data(), m_crypto_key.size(),
							  buffer.data(), buffer.size());
					std::lock_guard<std::mutex> lg(m_udp_mutex);
                    size_t udp_sended = m_udp->send(buffer.data(), encypted_message_size, m_dst_addr);
                    buffer.release();
				});
		} else {
			size_t encypted_message_size =
				m_crypto->encrypt(
					  buffer.data(), tun_read_size,
					  m_crypto_key.data(), m_crypto_key.size(),
					  buffer.data(), buffer.size());
            size_t udp_sended = m_udp->send(buffer.data(), encypted_message_size, m_dst_addr);
            buffer.release();
		}
    }
}

void node::run_async_tun(size_t number_of_tun_threads) {

	cBufferManager bufferManager(number_of_tun_threads, 1500 * 20); // for possible weld data, 1 buffer for 1 thread
	std::vector<std::thread> thread_vector;
	
	for (size_t i = 0; i < number_of_tun_threads; i++) {
		cBuffer & buffer = m_buffer_manager.get_free_buffer_or_wait();
		m_tun_async->async_read_from_tun(buffer.data(), buffer.size(), // call before threads, mutex is not needed
			[&buffer, this](size_t tun_read_size) {
				async_read_from_tun_handler(tun_read_size, buffer);
			}
		);
	}
	
	for (size_t i = 0; i < number_of_tun_threads; i++) {
		thread_vector.emplace_back([this]{
			m_tun_async->run(); // this is thread safe, mutex is not needed
		});
	}
	for (auto & thread : thread_vector)
		thread.join();
}

void node::run_multithread_sync(size_t number_of_tun_threads) {
	std::vector<std::thread> thread_vector;
	for (size_t i = 0; i < number_of_tun_threads; i++) {
		thread_vector.emplace_back([this]{
			std::vector<unsigned char> buffer(1500 * 20); // for possible weld data
			size_t tun_read_size = 0;
			while (true) {
				{
					std::lock_guard<std::mutex> lock(m_tun_async_mtx);
					tun_read_size = m_tun->read_from_tun(buffer.data(), buffer.size());
				}
				
				size_t encypted_message_size = m_crypto->encrypt(
								  buffer.data(), tun_read_size,
								  m_crypto_key.data(), m_crypto_key.size(),
								  buffer.data(), buffer.size());
				{
					std::lock_guard<std::mutex> lock(m_udp_mutex);
					m_udp->send(buffer.data(), encypted_message_size, m_dst_addr);
				}
			}
		});
	}
		
	for (auto & thread : thread_vector)
		thread.join();
}

void node::async_read_from_tun_handler(size_t tun_read_size, cBuffer & buffer) {
	size_t encypted_message_size =
		m_crypto->encrypt(
			  buffer.data(), tun_read_size,
			  m_crypto_key.data(), m_crypto_key.size(),
			  buffer.data(), buffer.size());
	{
		std::lock_guard<std::mutex> lg(m_udp_mutex);
		size_t udp_sended = m_udp->send(buffer.data(), encypted_message_size, m_dst_addr);
	}
	// continue reading
	std::lock_guard<std::mutex> lock(m_tun_async_mtx);
	m_tun_async->async_read_from_tun(buffer.data(), buffer.size(),
		[&buffer, this](size_t tun_read_size) {
			async_read_from_tun_handler(tun_read_size, buffer);
		}
	);
}
