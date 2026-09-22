#include "jsonrpc.hpp"

#include "../pluginstate.hpp"
#include "../types/pojson.hpp"

using porla::Lua::LuaState;
using porla::Lua::Packages::JsonRpc;

struct LuaMethod : public porla::Rpc::Method
{
    explicit LuaMethod(std::size_t callback_id, std::weak_ptr<LuaState> state)
        : m_callback_id(callback_id)
        , m_state(state)
    {
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
    std::size_t             m_callback_id;
    std::weak_ptr<LuaState> m_state;
};

sol::object JsonRpc::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("register", [](sol::this_state ts, const std::string& method, sol::main_protected_function callback)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr) { return; }

        auto jsonrpc = state->jsonrpc.lock();
        if (jsonrpc == nullptr) { return; }

        const auto callback_id = state->RegisterCallback(callback, false);

        if (!jsonrpc->Register(method, std::make_shared<LuaMethod>(callback_id, state)))
        {
            state->RemoveCallback(callback_id);
            throw sol::error("jsonrpc method '" + method + "' is already registered");
        }

        state->destructors.emplace_back([weak, method]()
        {
            auto state = weak.lock();
            if (!state) { return; }

            auto jsonrpc = state->jsonrpc.lock();
            if (!jsonrpc) { return; }

            jsonrpc->Unregister(method);
        });
    });

    return tbl;
}
