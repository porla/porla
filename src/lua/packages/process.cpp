#include "process.hpp"

#include <boost/log/trivial.hpp>
#include <boost/process.hpp>

#include "../plugins/plugin.hpp"

namespace bp = boost::process;
using porla::Lua::Packages::Process;
using porla::Lua::Plugins::PluginLoadOptions;

struct LaunchState
{
    bp::child c;
    sol::function callback;
    std::future<std::string> std_err;
    std::future<std::string> std_out;
};

static void Launch(sol::this_state s, const sol::table& args)
{
    sol::state_view lua{s};
    const auto options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();

    bp::environment env = boost::this_process::environment();

    bp::opstream std_in;

    auto state = std::make_shared<LaunchState>();
    state->callback = args["done"];
    state->c = bp::child(
        options.io,
        args["file"].get<std::string>(),
        bp::args(args["args"].get<std::vector<std::string>>()),
        bp::env(env),
        bp::start_dir(""),
        bp::std_in  < std_in,
        bp::std_err > state->std_err,
        bp::std_out > state->std_out,
        bp::on_exit(
            [state](int exit_code, const boost::system::error_code& ec)
            {
                state->callback(exit_code, state->std_out.get(), state->std_err.get());
            }));

    std_in.close();
}

void Process::Register(sol::state& lua)
{
    lua["package"]["preload"]["process"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table log = lua.create_table();

        log["launch"] = &Launch;

        return log;
    };
}
