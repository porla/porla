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
        "error", [](ResponseWriter& w, int code, const std::string& message)
        {
            w.Error(code, message);
        },
        "ok", [](ResponseWriter& w, sol::object data, sol::this_state ts)
        {
            const auto parsed_json = PoJson::ToJson(ts, data, 0);
            w.Ok(parsed_json);
        }
    );
}
