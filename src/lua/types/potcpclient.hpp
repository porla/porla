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
        struct Options
        {
            std::optional<double> connect_timeout;
            std::optional<double> read_timeout;
            bool                  nodelay   = false;
            bool                  keepalive = false;
        };

        static void Register(sol::state& lua);
        static Options ReadOptions(const sol::table& options);

        PoTcpClient(const std::shared_ptr<LuaState>& lua_state, Options options);
        ~PoTcpClient();

        void Close();

    private:
        struct WriteOp { std::string data; std::optional<std::size_t> callback_id; };

        struct TlsOptions
        {
            bool                       verify = true;
            std::optional<std::string> ca_file;
            std::optional<std::string> cert_file;
            std::optional<std::string> key_file;
            std::optional<std::string> server_name;
        };

        void Connect(const std::string& host, int port, sol::main_protected_function callback);
        void ConnectComplete(std::size_t callback_id, const boost::system::error_code& ec2, const tcp::endpoint& endpoint);
        void Arm(std::optional<double> seconds);
        void Disarm();
        void HandshakeComplete(std::size_t callback_id, const boost::system::error_code& ec);
        void Read(sol::main_protected_function callback);
        void ReadComplete(std::size_t callback_id, const boost::system::error_code& ec, std::size_t n);
        void ReadExactly(std::size_t n, sol::main_protected_function callback);
        void ResolveComplete(std::size_t callback_id, const boost::system::error_code& ec, const tcp::resolver::results_type& endpoints);
        void Tls(sol::main_protected_function callback);
        void TlsWith(const sol::table& options, sol::main_protected_function callback);
        void StartTls(const TlsOptions& options, sol::main_protected_function callback);
        void Write(const std::string& data, std::optional<sol::main_protected_function> callback);
        void WriteComplete(const boost::system::error_code& ec, std::size_t n);
        void WriteNext();
        sol::table ToPoError(lua_State* L, const boost::system::error_code& ec) const;

        bool                                                  m_closed = false;
        std::optional<std::string>                            m_connect_host;
        Options                                               m_options;
        bool                                                  m_peer_closed = false;
        std::vector<char>                                     m_read_buffer;
        bool                                                  m_read_pending = false;
        boost::asio::ip::tcp::resolver                        m_resolver;
        boost::asio::ip::tcp::socket                          m_socket;
        std::weak_ptr<LuaState>                               m_state;
        boost::asio::steady_timer                             m_timer;
        std::uint64_t                                         m_timer_generation = 0;
        bool                                                  m_timed_out = false;
        std::shared_ptr<boost::asio::ssl::context>            m_tls_ctx;
        std::optional<boost::asio::ssl::stream<tcp::socket&>> m_tls;
        std::deque<WriteOp>                                   m_write_buffer;
        bool                                                  m_write_pending = false;
    };
}
