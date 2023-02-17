#pragma once

#include <boost/asio.hpp>

#include "../action.hpp"

namespace porla::Lua::Workflows::Actions
{
    struct PushNtfyOptions
    {
        boost::asio::io_context& io;
        std::string              topic;
        sol::object              message;
    };

    class PushNtfy : public Action
    {
    public:
        explicit PushNtfy(const PushNtfyOptions& opts);

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        PushNtfyOptions m_opts;
    };
}
