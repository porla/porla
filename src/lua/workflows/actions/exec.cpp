#include "exec.hpp"

#include <map>

#include <boost/log/trivial.hpp>
#include <boost/process.hpp>
#include <utility>

namespace bp = boost::process;

using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::Actions::Exec;
using porla::Lua::Workflows::Actions::ExecOptions;

struct ExecInput
{
    std::string                                       file{};
    std::optional<std::vector<sol::object>>           args{};
    std::optional<std::map<std::string, std::string>> env{};
    std::optional<std::string>                        working_dir{};
};

struct Exec::ExecState
{
    bp::child c;
    std::future<std::string> std_err;
    std::future<std::string> std_out;
};

Exec::Exec(ExecOptions opts)
    : m_opts(std::move(opts))
{
}

Exec::~Exec() = default;

void Exec::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    bp::environment env = boost::this_process::environment();

    const ExecInput input;

    if (const auto input_env = input.env)
    {
        for (const auto &[key, val]: *input_env)
        {
            env[key] = val;
        }
    }

    bp::opstream std_in;

    if (m_opts.std_in.is<std::string>())
    {
        std_in << m_opts.std_in.as<std::string>();
    }
    else if (m_opts.std_in.is<std::function<std::string(sol::table)>>())
    {
        std_in << m_opts.std_in.as<std::function<std::string(sol::table)>>()(params.context);
    }

    std::vector<std::string> args;

    for (auto& arg : m_opts.args)
    {
        if (arg.is<std::string>())
        {
            args.push_back(arg.as<std::string>());
        }
        else if (arg.is<std::function<std::string(sol::table)>>())
        {
            args.push_back(arg.as<std::function<std::string(sol::table)>>()(params.context));
        }
        else
        {
            BOOST_LOG_TRIVIAL(warning) << "Exec argument is not a string or function";
        }
    }

    auto state = std::make_shared<ExecState>();
    state->c = bp::child(
        m_opts.io,
        m_opts.file,
        bp::args(args),
        bp::env(env),
        bp::start_dir(input.working_dir.value_or("")),
        bp::std_in  < std_in,
        bp::std_err > state->std_err,
        bp::std_out > state->std_out,
        bp::on_exit(
            [params, state, callback](int exit_code, const boost::system::error_code& ec)
            {
                sol::table output   = params.state.create_table();
                output["exit_code"] = exit_code;
                output["std_err"]   = state->std_err.get();
                output["std_out"]   = state->std_out.get();

                callback->Complete(output);
            }));

    std_in.close();
}
