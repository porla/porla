#include "exec.hpp"

#include <boost/log/trivial.hpp>
#include <boost/process.hpp>

namespace bp = boost::process;

using porla::Workflows::ActionCallback;
using porla::Workflows::Actions::Exec;

struct Exec::ExecState
{
    bp::child c;
    std::future<std::string> std_err;
    std::future<std::string> std_out;
};

Exec::Exec(boost::asio::io_context& io)
    : m_io(io)
{
}

Exec::~Exec() = default;

void Exec::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    auto state = std::make_shared<ExecState>();
    state->c = bp::child(
        m_io,
        params.Input()["file"].get<std::string>(),
        params.Input()["args"].get<std::string>(),
        bp::std_err > state->std_err,
        bp::std_out > state->std_out,
        bp::on_exit(
            [state, callback](int exit_code, const boost::system::error_code& ec)
            {
                nlohmann::json std_err_val;
                nlohmann::json std_out_val;

                if (state->std_err.valid())
                {
                    std_err_val = state->std_err.get();
                }

                if (state->std_out.valid())
                {
                    std_out_val = state->std_out.get();
                }

                callback->Complete({
                    {"exit_code", exit_code},
                    {"std_err", std_err_val},
                    {"std_out", std_out_val}
                });
            }));
}
