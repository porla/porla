#pragma once

#include "../action.hpp"

#include <boost/asio.hpp>

namespace porla::Lua::Workflows::Actions
{
    struct SleepOptions
    {
        boost::asio::io_context& io;
        int                      timeout;
    };

    class Sleep : public Action
    {
    public:
        explicit Sleep(const SleepOptions& opts);

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        SleepOptions m_opts;
    };
}
