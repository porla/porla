#include "pojsonrpcresponse.hpp"

#include "../../rpc/responsewriter.hpp"

#include "pojson.hpp"

using porla::Lua::Types::PoJsonRpcResponse;
using porla::Rpc::ResponseWriter;

void PoJsonRpcResponse::Register(sol::state& lua)
{
    lua.new_usertype<ResponseWriter>(
        "PoJsonRpcResponse",
        sol::no_constructor,
        "error", sol::overload(
            [](ResponseWriter& w, int code, const std::string& message)
            {
                w.Error(code, message);
            },
            [](ResponseWriter& w, int code, const std::string& message, const sol::object& data, sol::this_state ts)
            {
                w.Error(code, message, PoJson::ToJson(ts, data, 0));
            },
            [](ResponseWriter& w, const sol::table& err)
            {
                const auto code = porla::Errors::Find(err.get_or<std::string>("code", ""))
                    .value_or(porla::Errors::Code::internal_error);

                nlohmann::json data = nlohmann::json::object();

                if (const sol::optional<std::string> category = err["category"]) { data["category"] = *category; }
                if (const sol::optional<int> value = err["value"])               { data["value"]    = *value; }

                w.Error(code, err.get_or<std::string>("message", ""), data);
            }),
        "ok", [](ResponseWriter& w, sol::object data, sol::this_state ts)
        {
            const auto parsed_json = PoJson::ToJson(ts, data, 0);
            w.Ok(parsed_json);
        }
    );
}
