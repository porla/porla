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
        "close",        &PoTcpClient::Close,
        "connect",      &PoTcpClient::Connect,
        "read",         &PoTcpClient::Read,
        "read_exactly", &PoTcpClient::ReadExactly,
        "tls",          &PoTcpClient::Tls,
        "write",        &PoTcpClient::Write);
}

PoTcpClient::PoTcpClient(const std::shared_ptr<LuaState>& lua_state)
    : m_resolver(lua_state->io)
    , m_socket(lua_state->io)
    , m_state(lua_state)
    , m_tls_ctx(nullptr)
{
}

PoTcpClient::~PoTcpClient()
{
    Close();
}

void PoTcpClient::Close()
{
    if (m_closed)
    {
        return;
    }

    m_closed = true;

    m_resolver.cancel();
    m_socket.cancel();
    m_socket.close();
}

void PoTcpClient::Connect(const std::string& host, int port, sol::main_protected_function callback)
{
    auto state       = m_state.lock();
    auto callback_id = state->RegisterCallback(callback, true);

    m_connect_host = host;

    m_resolver.async_resolve(
        host,
        std::to_string(port),
        std::bind_front(&PoTcpClient::ResolveComplete, shared_from_this(), callback_id));
}

void PoTcpClient::ConnectComplete(std::size_t callback_id, const boost::system::error_code& ec, const tcp::endpoint& endpoint)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (ec)
    {
        state->InvokeCallback(callback_id, ec.message());
        return;
    }

    state->InvokeCallback(callback_id, sol::nil);
}

void PoTcpClient::HandshakeComplete(std::size_t callback_id, const boost::system::error_code& ec)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (ec)
    {
        state->InvokeCallback(callback_id, ec.message());
        return;
    }

    state->InvokeCallback(callback_id, sol::nil);
}

void PoTcpClient::Read(sol::main_protected_function callback)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (m_read_buffer.size() < 8192)
    {
        m_read_buffer.resize(8192);
    }

    const auto callback_id = state->RegisterCallback(callback, true);
    const auto handler     = std::bind_front(&PoTcpClient::ReadComplete, shared_from_this(), callback_id);
    const auto buffer      = boost::asio::buffer(m_read_buffer);

    if (m_tls_ctx && m_tls.has_value())
    {
        m_tls->async_read_some(buffer, handler);
    }
    else
    {
        m_socket.async_read_some(buffer, handler);
    }
}

void PoTcpClient::ReadComplete(std::size_t callback_id, const boost::system::error_code& ec, std::size_t n)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (ec)
    {
        state->InvokeCallback(callback_id, ec.message(), sol::nil);
        return;
    }

    std::string data(m_read_buffer.data(), n);

    state->InvokeCallback(callback_id, sol::nil, data);
}

void PoTcpClient::ReadExactly(std::size_t n, sol::main_protected_function callback)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    m_read_buffer.resize(n);
    m_read_pending = true;

    const auto callback_id = state->RegisterCallback(callback, true);
    const auto handler     = std::bind_front(&PoTcpClient::ReadComplete, shared_from_this(), callback_id);
    const auto buffer      = boost::asio::buffer(m_read_buffer);

    if (m_tls_ctx && m_tls.has_value())
    {
        boost::asio::async_read(
            m_tls.value(),
            buffer,
            boost::asio::transfer_exactly(n),
            handler);
    }
    else
    {
        boost::asio::async_read(
            m_socket,
            buffer,
            boost::asio::transfer_exactly(n),
            handler);
    }
}

void PoTcpClient::ResolveComplete(std::size_t callback_id, const boost::system::error_code& ec, const tcp::resolver::results_type& endpoints)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (ec)
    {
        state->InvokeCallback(callback_id, ec.message());
        return;
    }

    boost::asio::async_connect(
        m_socket,
        endpoints,
        std::bind_front(&PoTcpClient::ConnectComplete, shared_from_this(), callback_id));
}

void PoTcpClient::Tls(sol::main_protected_function callback)
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        return;
    }

    if (!m_connect_host.has_value())
    {
        BOOST_LOG_TRIVIAL(error) << "lua[PoTcpClient] Cannot do TLS upgrade due to missing connect host";
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
    m_tls->set_verify_callback(boost::asio::ssl::host_name_verification(m_connect_host.value()), ec);

    SSL_set_tlsext_host_name(m_tls->native_handle(), m_connect_host.value().c_str());

    auto callback_id = state->RegisterCallback(callback, true);

    m_tls->async_handshake(
        boost::asio::ssl::stream_base::client,
        std::bind_front(&PoTcpClient::HandshakeComplete, shared_from_this(), callback_id));
}

void PoTcpClient::Write(const std::string& data, std::optional<sol::main_protected_function> callback)
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        return;
    }

    std::optional<std::size_t> callback_id;
    
    if (callback.has_value())
    {
        callback_id = state->RegisterCallback(callback.value(), true);
    }

    m_write_buffer.push_back({ data, callback_id });

    boost::asio::post(state->io, std::bind(&PoTcpClient::WriteNext, shared_from_this()));
}

void PoTcpClient::WriteComplete(const boost::system::error_code& ec, std::size_t n)
{
    auto write_op = std::move(m_write_buffer.front());

    m_write_buffer.pop_front();
    m_write_pending = false;

    auto state = m_state.lock();

    if (state == nullptr)
    {
        return;
    }

    if (write_op.callback_id.has_value())
    {
        state->InvokeCallback(write_op.callback_id.value());
    }

    boost::asio::post(state->io, std::bind(&PoTcpClient::WriteNext, shared_from_this()));
}

void PoTcpClient::WriteNext()
{
    if (m_write_pending)
    {
        return;
    }

    if (m_write_buffer.empty())
    {
        m_write_pending = false;
        return;
    }

    m_write_pending = true;

    const auto& next_write = m_write_buffer.front();
    const auto  buffer     = boost::asio::buffer(next_write.data);
    const auto  handler    = std::bind_front(&PoTcpClient::WriteComplete, shared_from_this());

    if (m_tls_ctx && m_tls.has_value())
    {
        boost::asio::async_write(
            m_tls.value(),
            buffer,
            handler);
    }
    else
    {
        boost::asio::async_write(
            m_socket,
            buffer,
            handler);
    }
}
