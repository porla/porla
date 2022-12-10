#pragma once

#include <boost/asio.hpp>

namespace porla
{
    class ISession;

    class WebhookClient
    {
    public:
        explicit WebhookClient(boost::asio::io_context& io, ISession& session);

    private:
        boost::asio::io_context& m_io;
        ISession& m_session;
    };
}
