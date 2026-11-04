#include "potcpclient.hpp"

#include <chrono>

#include <boost/log/trivial.hpp>
#include <openssl/ssl.h>

#include "../pluginstate.hpp"

using porla::Lua::Types::PoTcpClient;
using PoTcpClientPtr = std::shared_ptr<PoTcpClient>;

namespace
{
    std::string KeyOf(const sol::object& key, const char* what)
    {
        if (key.get_type() != sol::type::string)
        {
            throw sol::error(std::string(what) + ": option keys must be strings");
        }

        return key.as<std::string>();
    }

    double Seconds(const sol::object& value, const std::string& key)
    {
        if (value.get_type() != sol::type::number || value.as<double>() <= 0)
        {
            throw sol::error("tcp_client: " + key + " must be a number of seconds > 0");
        }

        return value.as<double>();
    }

    bool Flag(const sol::object& value, const char* what, const std::string& key)
    {
        if (value.get_type() != sol::type::boolean)
        {
            throw sol::error(std::string(what) + ": " + key + " must be a boolean");
        }

        return value.as<bool>();
    }
}

PoTcpClient::Options PoTcpClient::ReadOptions(const sol::table& options)
{
    Options result;

    for (const auto& [k, v] : options)
    {
        const auto key = KeyOf(k, "tcp_client");

        if      (key == "connect_timeout") result.connect_timeout = Seconds(v, key);
        else if (key == "read_timeout")    result.read_timeout    = Seconds(v, key);
        else if (key == "nodelay")         result.nodelay         = Flag(v, "tcp_client", key);
        else if (key == "keepalive")       result.keepalive       = Flag(v, "tcp_client", key);
        else throw sol::error("tcp_client: unknown option '" + key + "'");
    }

    return result;
}

void PoTcpClient::Register(sol::state& lua)
{
    lua.new_usertype<PoTcpClient>(
        "PoTcpClient",
        sol::no_constructor,
        "close",        &PoTcpClient::Close,
        "connect",      &PoTcpClient::Connect,
        "read",         &PoTcpClient::Read,
        "read_exactly", &PoTcpClient::ReadExactly,
        "tls",          sol::overload(&PoTcpClient::Tls, &PoTcpClient::TlsWith),
        "write",        &PoTcpClient::Write);
}

PoTcpClient::PoTcpClient(const std::shared_ptr<LuaState>& lua_state, Options options)
    : m_options(options)
    , m_resolver(lua_state->io)
    , m_socket(lua_state->io)
    , m_state(lua_state)
    , m_timer(lua_state->io)
    , m_tls_ctx(nullptr)
{
}

PoTcpClient::~PoTcpClient()
{
    try
    {
        Close();
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(warning) << "lua[PoTcpClient] Error when closing socket: " << e.what();
    }
}

void PoTcpClient::Close()
{
    if (m_closed)
    {
        return;
    }

    m_closed = true;

    boost::system::error_code ec;

    Disarm();

    m_resolver.cancel();
    m_socket.cancel(ec);
    m_socket.close(ec);
}

void PoTcpClient::Connect(const std::string& host, int port, sol::main_protected_function callback)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    auto callback_id = state->RegisterCallback(callback, true);

    m_connect_host = host;

    Arm(m_options.connect_timeout);

    m_resolver.async_resolve(
        host,
        std::to_string(port),
        std::bind_front(&PoTcpClient::ResolveComplete, shared_from_this(), callback_id));
}

void PoTcpClient::ConnectComplete(std::size_t callback_id, const boost::system::error_code& ec, const tcp::endpoint& endpoint)
{
    Disarm();

    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (ec)
    {
        state->InvokeCallback(callback_id, ErrorText(ec));
        return;
    }

    boost::system::error_code opt_ec;

    if (m_options.nodelay)   m_socket.set_option(tcp::no_delay(true), opt_ec);
    if (m_options.keepalive) m_socket.set_option(boost::asio::socket_base::keep_alive(true), opt_ec);

    if (opt_ec)
    {
        BOOST_LOG_TRIVIAL(warning) << "lua[PoTcpClient] Failed to set socket options: " << opt_ec.message();
    }

    state->InvokeCallback(callback_id, sol::lua_nil);
}

void PoTcpClient::Arm(std::optional<double> seconds)
{
    const auto generation = ++m_timer_generation;

    if (!seconds.has_value())
    {
        return;
    }

    m_timer.expires_after(std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<double>(*seconds)));

    // the generation check stops a timer that fired just as its operation completed from closing the client
    m_timer.async_wait(
        [weak = weak_from_this(), generation](const boost::system::error_code& ec)
        {
            auto self = weak.lock();

            if (ec || self == nullptr || self->m_timer_generation != generation)
            {
                return;
            }

            self->m_timed_out = true;
            self->Close();
        });
}

void PoTcpClient::Disarm()
{
    ++m_timer_generation;
    m_timer.cancel();
}

std::string PoTcpClient::ErrorText(const boost::system::error_code& ec) const
{
    return m_timed_out ? "timed out" : ec.message();
}

void PoTcpClient::HandshakeComplete(std::size_t callback_id, const boost::system::error_code& ec)
{
    Disarm();

    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (ec)
    {
        state->InvokeCallback(callback_id, ErrorText(ec));
        return;
    }

    state->InvokeCallback(callback_id, sol::lua_nil);
}

void PoTcpClient::Read(sol::main_protected_function callback)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (m_read_pending)
    {
        throw sol::error("a read operation is already in progress");
    }

    if (m_read_buffer.size() < 8192)
    {
        m_read_buffer.resize(8192);
    }

    m_read_pending = true;

    Arm(m_options.read_timeout);

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
    m_read_pending = false;

    Disarm();

    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (ec)
    {
        state->InvokeCallback(callback_id, ErrorText(ec), sol::lua_nil);
        return;
    }

    std::string data(m_read_buffer.data(), n);

    state->InvokeCallback(callback_id, sol::lua_nil, data);
}

void PoTcpClient::ReadExactly(std::size_t n, sol::main_protected_function callback)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    if (m_read_pending)
    {
        throw sol::error("a read operation is already in progress");
    }

    m_read_buffer.resize(n);
    m_read_pending = true;

    Arm(m_options.read_timeout);

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
        Disarm();

        state->InvokeCallback(callback_id, ErrorText(ec));
        return;
    }

    boost::asio::async_connect(
        m_socket,
        endpoints,
        std::bind_front(&PoTcpClient::ConnectComplete, shared_from_this(), callback_id));
}

void PoTcpClient::Tls(sol::main_protected_function callback)
{
    StartTls({}, std::move(callback));
}

void PoTcpClient::TlsWith(const sol::table& options, sol::main_protected_function callback)
{
    TlsOptions tls;

    for (const auto& [k, v] : options)
    {
        const auto key = KeyOf(k, "tls");

        if (key == "verify")
        {
            tls.verify = Flag(v, "tls", key);
        }
        else if (key == "ca_file" || key == "cert_file" || key == "key_file" || key == "server_name")
        {
            if (v.get_type() != sol::type::string)
            {
                throw sol::error("tls: " + key + " must be a string");
            }

            auto& field = key == "ca_file"   ? tls.ca_file
                        : key == "cert_file" ? tls.cert_file
                        : key == "key_file"  ? tls.key_file
                        :                      tls.server_name;

            field = v.as<std::string>();
        }
        else
        {
            throw sol::error("tls: unknown option '" + key + "'");
        }
    }

    StartTls(tls, std::move(callback));
}

void PoTcpClient::StartTls(const TlsOptions& options, sol::main_protected_function callback)
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        return;
    }

    const auto server_name = options.server_name ? options.server_name : m_connect_host;

    if (!server_name.has_value())
    {
        BOOST_LOG_TRIVIAL(error) << "lua[PoTcpClient] Cannot do TLS upgrade due to missing connect host";
        state->InvokeCallback(state->RegisterCallback(callback, true), "missing connect host");
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

    if (options.ca_file)
    {
        m_tls_ctx->load_verify_file(*options.ca_file, ec);

        if (ec)
        {
            state->InvokeCallback(state->RegisterCallback(callback, true), "ca_file: " + ec.message());
            return;
        }
    }
    else
    {
        m_tls_ctx->set_default_verify_paths(ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(warning) << "lua[PoTcpClient] set_default_verify_paths failed: " << ec.message();
        }
    }

    if (options.key_file && !options.cert_file)
    {
        state->InvokeCallback(state->RegisterCallback(callback, true), "key_file needs cert_file");
        return;
    }

    if (options.cert_file)
    {
        m_tls_ctx->use_certificate_chain_file(*options.cert_file, ec);

        if (ec)
        {
            state->InvokeCallback(state->RegisterCallback(callback, true), "cert_file: " + ec.message());
            return;
        }

        m_tls_ctx->use_private_key_file(options.key_file.value_or(*options.cert_file), boost::asio::ssl::context::pem, ec);

        if (ec)
        {
            state->InvokeCallback(state->RegisterCallback(callback, true), "key_file: " + ec.message());
            return;
        }
    }

    m_tls.emplace(m_socket, *m_tls_ctx);

    m_tls->set_verify_mode(
        options.verify
            ? boost::asio::ssl::verify_peer
            : boost::asio::ssl::verify_none, ec);

    if (ec)
    {
        state->InvokeCallback(state->RegisterCallback(callback, true), ec.message());
        return;
    }

    if (options.verify)
    {
        m_tls->set_verify_callback(boost::asio::ssl::host_name_verification(server_name.value()), ec);
    }

    SSL_set_tlsext_host_name(m_tls->native_handle(), server_name.value().c_str());

    auto callback_id = state->RegisterCallback(callback, true);

    Arm(m_options.connect_timeout);

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
        ec  ? state->InvokeCallback(write_op.callback_id.value(), ec.message())
            : state->InvokeCallback(write_op.callback_id.value(), sol::lua_nil);
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
