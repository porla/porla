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

        ~PoHttpServerResponse();

        void Setup();

    private:
        void BuildForm(LuaState& state);
        bool ParseMultipart(LuaState& state, sol::table& fields, sol::table& files);

        void Fail();
        void Finish();
        void Finish(const std::string& data);
        void OnData(std::string_view data, std::uint64_t len);
        void Write(const std::string& data);
        void WriteHeader(const std::string& key, const std::string& value);
        void WriteStatus(const std::string& status);

        std::shared_ptr<std::string> m_body;
        std::size_t                  m_callback_id;
        std::string                  m_content_type;
        bool                         m_is_aborted;
        bool                         m_is_finished;
        bool                         m_is_started;
        sol::table                   m_request;
        uWS::HttpResponse<false>*    m_response;
        std::weak_ptr<LuaState>      m_state;
    };
}
