#pragma once

#include <boost/asio.hpp>

#include "../action.hpp"

namespace porla::Lua::Workflows::Actions
{
    struct PushDiscordOptions
    {
        boost::asio::io_context& io;
        std::string              url;
        sol::object              message;
    };

    class PushDiscord : public porla::Lua::Workflows::Action
    {
    public:
        explicit PushDiscord(const PushDiscordOptions& opts);

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        PushDiscordOptions m_opts;
    };
}
