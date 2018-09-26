#ifndef NODE_H
#define NODE_H

#include "iCrypto.h"
#include "iTun.h"
#include "iUdp.h"
#include "thread_pool.h"
#include <boost/asio/io_service.hpp>
#include "cbuffermanager.h"
#include <memory>
#include <mutex>
#include <sodium.h>

class node final {
    public:
        node();
        void run(); ///< sync tun
        void run_async_tun(size_t number_of_tun_threads); ///< multiple tun threads
        void run_multithread_sync(size_t number_of_tun_threads);
		void run_multiqueue_sync(size_t number_of_tun_threads);
		void run_sync_receive();
        node(node &&) = default;
    private:
        boost::asio::ip::address m_dst_addr;
        std::unique_ptr<boost::asio::io_service> m_io_service;
        std::unique_ptr<iCrypto> m_crypto;
        std::unique_ptr<iTun> m_tun;
        std::unique_ptr<iTunAsync> m_tun_async;
        std::mutex m_tun_async_mtx;
        std::unique_ptr<iUdp> m_udp;
        std::mutex m_udp_mutex;
        std::unique_ptr<ThreadPool> m_thread_pool;
        cBufferManager m_buffer_manager;
        std::array<unsigned char, crypto_secretbox_KEYBYTES> m_crypto_key; // TODO: size
        friend class cNode_factory;
		
		void async_read_from_tun_handler(size_t tun_read_size, cBuffer & buffer);
};

#endif // NODE_H

