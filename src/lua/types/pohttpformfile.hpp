#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    class PoHttpFormFile
    {
    public:
        static void Register(sol::state& lua);

        PoHttpFormFile(std::shared_ptr<std::string> owner,
                       std::string name,
                       std::string filename,
                       std::string content_type,
                       std::string_view content);

    private:
        std::shared_ptr<std::string> m_owner;
        std::string      m_name;
        std::string      m_filename;
        std::string      m_content_type;
        std::string_view m_content;
    };
}
