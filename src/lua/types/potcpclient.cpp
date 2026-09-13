#include "potcpclient.hpp"

#include <boost/log/trivial.hpp>
#include <openssl/ssl.h>

#include "../pluginstate.hpp"

using porla::Lua::Types::PoTcpClient;
using PoTcpClientPtr = std::shared_ptr<PoTcpClient>;

void PoTcpClient::Register(sol::state& lua)
{
    lua.new_usertype<PoTcpClient>(
        "PoTcpClient",
        sol::no_constructor,
        "connect", &PoTcpClient::Connect,
        "read",    &PoTcpClient::Read,
        "tls",     &PoTcpClient::Tls);
}

PoTcpClient::PoTcpClient(const std::shared_ptr<LuaState>& lua_state)
    : m_io(lua_state->io)
    , m_resolver(lua_state->io)
    , m_socket(lua_state->io)
    , m_state(lua_state)
    , m_tls_ctx(nullptr)
{
}

void PoTcpClient::Connect(const std::string& host, int port, sol::main_protected_function callback)
{
    auto state = m_state.lock();

    auto callback_id = state->RegisterCallback(callback, true);

    m_resolver.async_resolve(
        host,
        std::to_string(port),
        std::bind_front(&PoTcpClient::ResolveComplete, shared_from_this(), callback_id));
}

void PoTcpClient::ConnectComplete(std::size_t callback_id, const boost::system::error_code& ec, const tcp::endpoint& endpoint)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "lua[PoTcpClient] Failed to connect: " << ec;
        return;
    }

    auto state = m_state.lock();

    if (state == nullptr)
    {
        BOOST_LOG_TRIVIAL(trace) << "lua[PoTcpClient] Failed to lock state";
        return;
    }

    state->InvokeCallback(callback_id);
}

void PoTcpClient::HandshakeComplete(std::size_t callback_id, const boost::system::error_code& ec)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "lua[PoTcpClient] TLS handshake failed: " << ec;
        return;
    }

    auto state = m_state.lock();

    if (state == nullptr)
    {
        return;
    }

    state->InvokeCallback(callback_id);
}

void PoTcpClient::ResolveComplete(std::size_t callback_id, const boost::system::error_code& ec, const tcp::resolver::results_type& endpoints)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "lua[PoTcpClient] Failed to resolve: " << ec;
        return;
    }

    boost::asio::async_connect(
        m_socket,
        endpoints,
        std::bind_front(&PoTcpClient::ConnectComplete, shared_from_this(), callback_id));
}

void PoTcpClient::Read(sol::main_protected_function callback)
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        return;
    }

    auto callback_id = state->RegisterCallback(callback, true);

    if (m_tls_ctx && m_tls.has_value())
    {
        m_tls->async_read_some(
            boost::asio::buffer(m_buffer),
            std::bind_front(&PoTcpClient::ReadComplete, shared_from_this(), callback_id));
    }
    else
    {
        m_socket.async_read_some(
            boost::asio::buffer(m_buffer),
            std::bind_front(&PoTcpClient::ReadComplete, shared_from_this(), callback_id));
    }
}

void PoTcpClient::ReadComplete(std::size_t callback_id, const boost::system::error_code& ec, std::size_t n)
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        return;
    }

    std::string data(m_buffer.data(), n);

    state->InvokeCallback(callback_id, data);
}

void PoTcpClient::Tls(sol::main_protected_function callback)
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        return;
    }

    boost::system::error_code ec;

    m_tls_ctx = std::make_shared<boost::asio::ssl::context>(
        boost::asio::ssl::context::tls_client);

    m_tls_ctx->set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::no_sslv3 |
        boost::asio::ssl::context::no_tlsv1 |
        boost::asio::ssl::context::no_tlsv1_1,
        ec);

    m_tls_ctx->set_default_verify_paths(ec);

    m_tls.emplace(m_socket, *m_tls_ctx);
    m_tls->set_verify_callback(boost::asio::ssl::host_name_verification("irc.libera.chat"), ec);

    SSL_set_tlsext_host_name(m_tls->native_handle(), "irc.libera.chat");

    auto callback_id = state->RegisterCallback(callback, true);

    m_tls->async_handshake(
        boost::asio::ssl::stream_base::client,
        std::bind_front(&PoTcpClient::HandshakeComplete, shared_from_this(), callback_id));
}
