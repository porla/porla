#pragma once

#include <deque>
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
        ~PoTcpClient();

    private:
        struct WriteOp { std::string data; std::optional<std::size_t> callback_id; };

        void Close();
        void Connect(const std::string& host, int port, sol::main_protected_function callback);
        void ConnectComplete(std::size_t callback_id, const boost::system::error_code& ec2, const tcp::endpoint& endpoint);
        void HandshakeComplete(std::size_t callback_id, const boost::system::error_code& ec);
        void Read(sol::main_protected_function callback);
        void ReadComplete(std::size_t callback_id, const boost::system::error_code& ec, std::size_t n);
        void ReadExactly(std::size_t n, sol::main_protected_function callback);
        void ResolveComplete(std::size_t callback_id, const boost::system::error_code& ec, const tcp::resolver::results_type& endpoints);
        void Tls(sol::main_protected_function callback);
        void Write(const std::string& data, std::optional<sol::main_protected_function> callback);
        void WriteComplete(const boost::system::error_code& ec, std::size_t n);
        void WriteNext();

        bool                                                  m_closed;
        std::optional<std::string>                            m_connect_host;
        std::vector<char>                                     m_read_buffer;
        bool                                                  m_read_pending;
        boost::asio::ip::tcp::resolver                        m_resolver;
        boost::asio::ip::tcp::socket                          m_socket;
        std::weak_ptr<LuaState>                               m_state;
        std::shared_ptr<boost::asio::ssl::context>            m_tls_ctx;
        std::optional<boost::asio::ssl::stream<tcp::socket&>> m_tls;
        std::deque<WriteOp>                                   m_write_buffer;
        bool                                                  m_write_pending;
    };
}
