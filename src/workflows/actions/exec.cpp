#include "exec.hpp"

#include <boost/log/trivial.hpp>
#include <boost/process.hpp>

namespace bp = boost::process;

using porla::Workflows::ActionCallback;
using porla::Workflows::Actions::Exec;

struct ExecInput
{
    std::string                                       file{};
    std::optional<std::vector<std::string>>           args{};
    std::optional<std::map<std::string, std::string>> env{};
    std::optional<std::string>                        working_dir{};
};

struct Exec::ExecState
{
    bp::child c;
    std::future<std::string> std_err;
    std::future<std::string> std_out;
};

static void from_json(const nlohmann::json& j, ExecInput& ei)
{
    j["file"].get_to(ei.file);

    if (j.contains("args"))
    {
        ei.args = j["args"].get<std::vector<std::string>>();
    }

    if (j.contains("env"))
    {
        ei.env = j["env"].get<std::map<std::string, std::string>>();
    }

    if (j.contains("working_dir"))
    {
        ei.working_dir = j["working_dir"].get<std::string>();
    }
}

Exec::Exec(boost::asio::io_context& io)
    : m_io(io)
{
}

Exec::~Exec() = default;

void Exec::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    bp::environment env = boost::this_process::environment();

    const ExecInput input = params.Input();

    if (const auto input_env = input.env)
    {
        for (const auto &[key, val]: *input_env)
        {
            env[key] = val;
        }
    }

    auto state = std::make_shared<ExecState>();
    state->c = bp::child(
        m_io,
        input.file,
        bp::args(input.args.value_or(std::vector<std::string>())),
        bp::env(env),
        bp::start_dir(input.working_dir.value_or("")),
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
