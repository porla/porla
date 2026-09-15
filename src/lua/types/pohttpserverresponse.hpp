#pragma once

#include <memory>

#include <sol/sol.hpp>
#include <uWebSockets/App.h>

namespace porla::Lua
{
    struct LuaState;
}

namespace porla::Lua::Types
{
    class PoHttpServerResponse : public std::enable_shared_from_this<PoHttpServerResponse>
    {
    public:
        static void Register(sol::state& lua);

        explicit PoHttpServerResponse(
            std::shared_ptr<LuaState> state,
            uWS::HttpRequest* req,
            uWS::HttpResponse<false>* response,
            std::size_t callback_id);

        void Setup();

    private:
        void Finish();
        void Finish(const std::string& data);
        void OnData(std::string_view data, std::uint64_t len);
        void Write(const std::string& data);
        void WriteStatus(const std::string& status);

        std::optional<std::string> m_body;
        std::size_t                m_callback_id;
        bool                       m_is_aborted;
        sol::table                 m_request;
        uWS::HttpResponse<false>*  m_response;
        std::weak_ptr<LuaState>    m_state;
    };
}
