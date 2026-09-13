#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <sol/sol.hpp>

using tcp = boost::asio::ip::tcp;

namespace porla::Lua
{
    struct LuaState;
}

namespace porla::Lua::Types
{
    class PoTcpClient : public std::enable_shared_from_this<PoTcpClient>
    {
    public:
        static void Register(sol::state& lua);

        explicit PoTcpClient(const std::shared_ptr<LuaState>& lua_state);

    private:
        void Connect(const std::string& host, int port, sol::main_protected_function callback);
        void ConnectComplete(std::size_t callback_id, const boost::system::error_code& ec2, const tcp::endpoint& endpoint);
        void HandshakeComplete(std::size_t callback_id, const boost::system::error_code& ec);
        void Read(sol::main_protected_function callback);
        void ReadComplete(std::size_t callback_id, const boost::system::error_code& ec, std::size_t n);
        void ResolveComplete(std::size_t callback_id, const boost::system::error_code& ec, const tcp::resolver::results_type& endpoints);
        void Tls(sol::main_protected_function callback);

        boost::asio::io_context&                              m_io;
        std::array<char, 8192>                                m_buffer;
        boost::asio::ip::tcp::resolver                        m_resolver;
        boost::asio::ip::tcp::socket                          m_socket;
        std::weak_ptr<LuaState>                               m_state;
        std::shared_ptr<boost::asio::ssl::context>            m_tls_ctx;
        std::optional<boost::asio::ssl::stream<tcp::socket&>> m_tls;
    };
}
