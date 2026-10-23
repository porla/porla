#pragma once

#include <set>
#include <string>
#include <string_view>
#include <vector>

#include <uWebSockets/App.h>

namespace porla
{
    // Stops DNS rebinding: a page on an attacker's domain that resolves to 127.0.0.1 is same-origin
    // for the browser and can read Porla's replies. Its requests carry the attacker's name in Host.
    //
    // When Porla listens on loopback (or allowed hosts are configured), only loopback names
    // (localhost, *.localhost, 127.0.0.0/8, ::1) and the allowed hosts are answered.

    class HttpHostGuard
    {
    public:
        HttpHostGuard(std::string_view bind_host, const std::vector<std::string>& allowed_hosts);

        bool Allows(std::string_view host_header) const;
        bool Enabled() const { return m_enabled; }
        bool Reject(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        bool                  m_enabled;
        std::set<std::string> m_allowed_hosts;
        std::set<std::string> m_logged;
    };
}
