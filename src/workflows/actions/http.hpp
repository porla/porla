#pragma once

#include <boost/asio.hpp>

#include "../action.hpp"

namespace porla::Workflows::Actions
{
    class Http : public Action
    {
    public:
        explicit Http(boost::asio::io_context& io);
        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        boost::asio::io_context& m_io;
    };
}