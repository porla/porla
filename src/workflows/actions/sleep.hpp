#pragma once

#include "../action.hpp"

namespace porla::Workflows::Actions
{
    class Sleep : public porla::Workflows::Action
    {
    public:
        explicit Sleep(boost::asio::io_context& io);

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        boost::asio::io_context& m_io;
    };
}
