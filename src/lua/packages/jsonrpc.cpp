#include "jsonrpc.hpp"

#include "../pluginstate.hpp"
#include "../types/pojson.hpp"
#include "../../rpc/jsonrpc.hpp"

using porla::Lua::LuaState;
using porla::Lua::Packages::JsonRpc;

struct LuaMethod : public porla::Rpc::Method
{
    explicit LuaMethod(std::size_t callback_id, std::weak_ptr<LuaState> state, std::optional<sol::main_protected_function> can_invoke)
        : m_callback_id(callback_id)
        , m_state(state)
        , m_can_invoke(can_invoke)
    {
    }

    bool CanInvoke(const porla::Auth::Context& ctx) override
    {
        if (m_state.expired())
        {
            return false;
        }

        if (!m_can_invoke.has_value())
        {
            return ctx.IsAuthenticated() && ctx.kind == porla::Auth::Context::Kind::User;
        }

        sol::protected_function_result result = m_can_invoke.value()(ctx);

        if (!result.valid())
        {
            sol::error err = result;
            BOOST_LOG_TRIVIAL(error) << "Failed to run CanInvoke callback for JSONRPC method: " << err.what();
            return false;
        }

        const auto allowed = result.get<sol::optional<bool>>();

        if (!allowed.has_value())
        {
            BOOST_LOG_TRIVIAL(error) << "CanInvoke callback for JSONRPC method did not return a boolean";
            return false;
        }

        return allowed.value();
    }

    void Invoke(const nlohmann::json& body, porla::Rpc::ResponseWriterHandle writer) override
    {
        auto state = m_state.lock();

        if (state == nullptr)
        {
            return writer->Error(-99, "Failed to lock state");
        }

        const auto lua_body = body.is_null()
            ? sol::object(state->lua.create_table())
            : porla::Lua::Types::PoJson::ToLua(state->lua.lua_state(), body, 0);

        state->InvokeCallback(m_callback_id, lua_body, writer);
    }

private:
    std::size_t                                 m_callback_id;
    std::weak_ptr<LuaState>                     m_state;
    std::optional<sol::main_protected_function> m_can_invoke;
};

sol::object JsonRpc::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("register", [](sol::this_state ts, const std::string& method, sol::main_protected_function callback,  std::optional<sol::main_protected_function> can_invoke)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr) { return; }

        auto jsonrpc = state->jsonrpc.lock();
        if (jsonrpc == nullptr) { return; }

        const auto callback_id = state->RegisterCallback(callback, false);

        if (!jsonrpc->Register(method, std::make_shared<LuaMethod>(callback_id, state, can_invoke)))
        {
            state->RemoveCallback(callback_id);
            throw sol::error("jsonrpc method '" + method + "' is already registered");
        }

        state->destructors.emplace_back([jsonrpc_weak = state->jsonrpc, method]()
        {
            auto jsonrpc = jsonrpc_weak.lock();
            if (!jsonrpc) { return; }

            jsonrpc->Unregister(method);
        });
    });

    return tbl;
}
