#include "httphostguard.hpp"

#include <algorithm>
#include <cctype>
#include <optional>

#include <boost/asio/ip/address.hpp>
#include <boost/log/trivial.hpp>

#include "utils/string.hpp"

using porla::HttpHostGuard;

namespace
{
    // "name", "name:port", "[v6]" or "[v6]:port" -> lowercase name without port, brackets or trailing
    // dot; nullopt when malformed (unclosed bracket, junk after it, a port that isn't digits)
    std::optional<std::string> HostName(std::string_view host)
    {
        std::string_view port;

        if (host.starts_with('['))
        {
            const auto end = host.find(']');

            if (end == std::string_view::npos)
            {
                return std::nullopt;
            }

            port = host.substr(end + 1);
            host = host.substr(1, end - 1);

            if (!port.empty() && !port.starts_with(':'))
            {
                return std::nullopt;
            }
        }
        else if (const auto colon = host.find(':'); colon != std::string_view::npos && host.find(':', colon + 1) == std::string_view::npos)
        {
            port = host.substr(colon);
            host = host.substr(0, colon);
        }

        if (!port.empty())
        {
            port.remove_prefix(1);

            if (port.empty() || !std::all_of(port.begin(), port.end(), [](unsigned char c) { return std::isdigit(c); }))
            {
                return std::nullopt;
            }
        }

        if (host.ends_with('.'))
        {
            host.remove_suffix(1);
        }

        if (host.empty())
        {
            return std::nullopt;
        }

        return porla::Utils::String::ToLower(host);
    }

    bool IsLoopbackName(const std::string& name)
    {
        if (name == "localhost" || name.ends_with(".localhost"))
        {
            return true;
        }

        boost::system::error_code ec;
        const auto address = boost::asio::ip::make_address(name, ec);

        return !ec && address.is_loopback();
    }
}

HttpHostGuard::HttpHostGuard(std::string_view bind_host, const std::vector<std::string>& allowed_hosts)
{
    bool any = false;

    for (const auto& host : allowed_hosts)
    {
        if (host == "*")
        {
            any = true;
        }
        else if (const auto name = HostName(host))
        {
            m_allowed_hosts.insert(*name);
        }
        else
        {
            BOOST_LOG_TRIVIAL(warning) << "Ignoring invalid allowed host '" << host << "'";
        }
    }

    const auto bind = HostName(bind_host);

    m_enabled = !any && (!m_allowed_hosts.empty() || (bind && IsLoopbackName(*bind)));
}

bool HttpHostGuard::Allows(std::string_view host_header) const
{
    // Browsers always send Host; a client without one (HTTP/1.0) can't be a rebinding page.
    if (!m_enabled || host_header.empty())
    {
        return true;
    }

    const auto name = HostName(host_header);

    return name && (IsLoopbackName(*name) || m_allowed_hosts.contains(*name));
}

bool HttpHostGuard::Reject(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    const auto host = req->getHeader("host");

    if (Allows(host))
    {
        return false;
    }

    // once per name (and not too many), so a flood can't fill the log
    if (const auto name = HostName(host).value_or(std::string(host.substr(0, 64))); m_logged.size() < 32 && m_logged.insert(name).second)
    {
        BOOST_LOG_TRIVIAL(warning)
            << "Rejected a request for host '" << name << "': porla only answers to localhost names here. "
            << "Behind a reverse proxy, add the name with --http-allowed-hosts or PORLA_HTTP_ALLOWED_HOSTS";
    }

    res->writeStatus("403 Forbidden")->end("Forbidden", true);
    return true;
}
