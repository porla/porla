#pragma once

#include "../../action.hpp"

namespace porla::Workflows::Actions::Push
{
    class Ntfy : public porla::Workflows::Action, public std::enable_shared_from_this<Ntfy>
    {
    public:
        explicit Ntfy(boost::asio::io_context& io);

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        boost::asio::io_context& m_io;
    };
}
