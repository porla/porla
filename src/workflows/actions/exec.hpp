#pragma once

#include <memory>
#include <vector>

#include "../action.hpp"

namespace porla::Workflows::Actions
{
    class Exec : public porla::Workflows::Action
    {
    public:
        explicit Exec(boost::asio::io_context& io);
        ~Exec();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        struct ExecState;

        boost::asio::io_context& m_io;
        std::vector<std::unique_ptr<ExecState>> m_state;
    };
}
