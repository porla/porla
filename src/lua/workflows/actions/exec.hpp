#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "../action.hpp"

namespace porla::Lua::Workflows::Actions
{
    struct ExecOptions
    {
        boost::asio::io_context& io;
        std::string              file;
        std::vector<std::string> args;
    };

    class Exec : public porla::Lua::Workflows::Action
    {
    public:
        explicit Exec(ExecOptions opts);
        ~Exec();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        struct ExecState;

        ExecOptions m_opts;
        std::vector<std::unique_ptr<ExecState>> m_state;
    };
}
