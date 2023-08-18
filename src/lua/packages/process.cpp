#include "../packages.hpp"

#include <boost/log/trivial.hpp>
#include <boost/process.hpp>

#include "../plugin.hpp"

namespace bp = boost::process;
using porla::Lua::Packages::Process;
using porla::Lua::PluginLoadOptions;

struct LaunchState
{
    bp::child c;
    sol::function callback;
    bp::ipstream std_err;
    bp::ipstream std_out;
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
                std::stringstream std_out;
                std_out << state->std_out.rdbuf();

                std::stringstream std_err;
                std_err << state->std_err.rdbuf();

                try
                {
                    state->callback(exit_code, std_out.str(), std_err.str());
                }
                catch (const std::exception& err)
                {
                    BOOST_LOG_TRIVIAL(error) << "Error when invoking process callback: " << err.what();
                }
            }));

    std_in.close();
}

void Process::Register(sol::state& lua)
{
    lua["package"]["preload"]["process"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table process = lua.create_table();

        process["launch"] = &Launch;

        process["search_path"] = [](const std::string& path)
        {
            return bp::search_path(path).string();
        };

        return process;
    };
}
