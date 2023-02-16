#include "engine.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <utility>

#include "workflows/action.hpp"
#include "workflows/actions/exec.hpp"
#include "workflows/actions/log.hpp"

#include "../session.hpp"

namespace fs = std::filesystem;

using porla::Lua::Engine;
using porla::Lua::EngineOptions;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionCallback;

struct Engine::State
{
    sol::state lua;
    boost::signals2::connection on_torrent_added;
};

struct LuaAction
{
    virtual std::shared_ptr<Action> Build(const EngineOptions& opts) = 0;
};

class ActionsExec : public LuaAction
{
public:
    explicit ActionsExec(sol::table args)
        : m_args(std::move(args))
    {
    }

    std::shared_ptr<Action> Build(const EngineOptions& opts) override
    {
        porla::Lua::Workflows::Actions::ExecOptions exec_opts{
            .io   = opts.io,
            .file = "/usr/bin/curl",
            .args = {"https://www.google.com"}
        };

        return std::make_shared<porla::Lua::Workflows::Actions::Exec>(exec_opts);
    }

private:
    sol::table m_args;
};

class ActionsLog : public LuaAction
{
public:
    explicit ActionsLog(sol::table args)
        : m_args(std::move(args))
    {
    }

    std::shared_ptr<Action> Build(const EngineOptions& opts) override
    {
        return std::make_shared<porla::Lua::Workflows::Actions::Log>(m_args);
    }

private:
    sol::table m_args;
};

struct ActionsSleep : public LuaAction
{
    explicit ActionsSleep(sol::table args)
    {
        BOOST_LOG_TRIVIAL(info) << args.size();
    }

    std::shared_ptr<Action> Build(const EngineOptions& opts) override
    {
        return nullptr;
    }
};

struct ActionsTorrentMove : public LuaAction
{
    explicit ActionsTorrentMove(sol::table args)
    {
        BOOST_LOG_TRIVIAL(info) << args.size();
    }

    std::shared_ptr<Action> Build(const EngineOptions& opts) override
    {
        return nullptr;
    }
};

class WorkflowExecutor : public ActionCallback, public std::enable_shared_from_this<WorkflowExecutor>
{
public:
    explicit WorkflowExecutor(EngineOptions opts, sol::table ctx, const std::vector<sol::object>& actions)
        : m_opts(std::move(opts))
        , m_ctx(std::move(ctx))
        , m_actions(actions)
    {
    }

    void Run()
    {
        if (m_actions.empty()) return;
        if (m_current_action >= m_actions.size()) return;

        BOOST_LOG_TRIVIAL(info) << "run";

        const auto& current_action = m_actions.at(m_current_action);

        BOOST_LOG_TRIVIAL(info) << "run 2";

        if (current_action.is<LuaAction*>())
        {
            BOOST_LOG_TRIVIAL(info) << "run 3";

            auto instance = current_action.as<LuaAction*>()->Build(m_opts);

            BOOST_LOG_TRIVIAL(info) << "run 4";

            porla::Lua::Workflows::ActionParams params{
                .context = m_ctx
            };

            BOOST_LOG_TRIVIAL(info) << "run 5";

            instance->Invoke(params, shared_from_this());

            BOOST_LOG_TRIVIAL(info) << "run 6";
        }
        else
        {
            BOOST_LOG_TRIVIAL(warning) << "A non-lua action found";
        }
    }

    void Complete() override
    {
        m_current_action++;

        boost::asio::post(
            m_opts.io,
            [_this = shared_from_this()]()
            {
                _this->Run();
            });
    }

private:
    EngineOptions m_opts;
    sol::table m_ctx;
    std::vector<sol::object> m_actions;
    size_t m_current_action{0};
};

class Workflow
{
public:
    explicit Workflow(const sol::table& args)
    {
        m_on = args["on"];

        BOOST_LOG_TRIVIAL(info) << sol::type_name(args.lua_state(), args["actions"].get_type());

        const sol::table& actions = args["actions"];

        for (const auto& item : actions)
        {
            BOOST_LOG_TRIVIAL(info) << sol::type_name(args.lua_state(), item.second.get_type());

            if (item.second.is<LuaAction*>())
            {
                m_actions.emplace_back(item.second);
            }
        }
    }

    ~Workflow()
    {
        BOOST_LOG_TRIVIAL(info) << "Destroying workflow";
    }

    std::string GetOn() { return m_on; }

    void Invoke(const EngineOptions& opts, const sol::table& ctx)
    {
        boost::asio::post(
            opts.io,
            [instance = std::make_shared<WorkflowExecutor>(opts, ctx, m_actions)]()
            {
                instance->Run();
            });
    }

private:
    std::string m_on;
    std::vector<sol::object> m_actions;
};

struct Engine::WorkflowInstance
{
    Workflow* workflow;
};

sol::table open_mylib(sol::this_state s)
{
    sol::state_view lua(s);

    sol::usertype<Workflow> workflow_type = lua.new_usertype<Workflow>(
        "Workflow",
        sol::constructors<Workflow(sol::table)>());

    workflow_type["on"] = sol::property(&Workflow::GetOn);

    sol::table module = lua.create_table();
    module["Workflow"] = workflow_type;
    module["Workflow"]["TorrentAdded"] = "TorrentAdded";

    return module["Workflow"];
}

sol::table OpenActionsExec(sol::this_state s)
{
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<ActionsExec>(
        "Exec",
        sol::constructors<ActionsExec(sol::table)>(),
        sol::base_classes,
        sol::bases<LuaAction>());

    return module["Exec"];
}

sol::table OpenActionsLog(sol::this_state s)
{
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<ActionsLog>(
        "Log",
        sol::constructors<ActionsLog(sol::table)>(),
        sol::base_classes,
        sol::bases<LuaAction>());

    return module["Log"];
}

sol::table OpenActionsSleep(sol::this_state s) {
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<ActionsSleep>(
            "Sleep",
            sol::constructors<ActionsSleep(sol::table)>(),
            sol::base_classes,
            sol::bases<LuaAction>());

    return module["Sleep"];
}

sol::table OpenActionsTorrentMove(sol::this_state s) {
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<ActionsTorrentMove>(
            "TorrentMove",
            sol::constructors<ActionsTorrentMove(sol::table)>(),
            sol::base_classes,
            sol::bases<LuaAction>());

    return module["TorrentMove"];
}

Engine::Engine(const EngineOptions& opts)
    : m_opts(opts)
{
    m_state = std::make_unique<State>();
    m_state->lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);

    m_state->lua.require("porla.Workflow", sol::c_call<decltype(&open_mylib), &open_mylib>);
    m_state->lua.require("porla.actions.Exec", sol::c_call<decltype(&OpenActionsExec), &OpenActionsExec>);
    m_state->lua.require("porla.actions.Log", sol::c_call<decltype(&OpenActionsLog), &OpenActionsLog>);
    m_state->lua.require("porla.actions.Sleep", sol::c_call<decltype(&OpenActionsSleep), &OpenActionsSleep>);
    m_state->lua.require("porla.actions.TorrentMove", sol::c_call<decltype(&OpenActionsTorrentMove), &OpenActionsTorrentMove>);

    for (const auto& file : fs::directory_iterator(opts.workflow_dir))
    {
        if (file.path().extension() != ".lua") continue;

        const auto workflow = m_state->lua.script_file(file.path());

        m_workflow_instances.emplace_back(WorkflowInstance{
            .workflow = workflow.get<Workflow*>()
        });
    }

    m_state->on_torrent_added = opts.session.OnTorrentAdded([=](auto && s) { OnTorrentAdded(s); });
}

Engine::~Engine() = default;

void Engine::OnTorrentAdded(const libtorrent::torrent_status& ts)
{
    BOOST_LOG_TRIVIAL(info) << ts.name;

    for (const auto& instance : m_workflow_instances)
    {
        sol::table ctx = m_state->lua.create_table();
        ctx["torrent"] = "hej";

        instance.workflow->Invoke(m_opts, ctx);
    }
}
