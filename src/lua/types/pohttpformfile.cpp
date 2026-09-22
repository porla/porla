#include "pohttpformfile.hpp"

using porla::Lua::Types::PoHttpFormFile;

void PoHttpFormFile::Register(sol::state& lua)
{
    lua.new_usertype<PoHttpFormFile>(
        "PoHttpFormFile",
        sol::no_constructor,
        "name", sol::property([](const PoHttpFormFile& f) { return f.m_name; })
    );
}

PoHttpFormFile::PoHttpFormFile(
    std::shared_ptr<std::string> owner,
    std::string name,
    std::string filename,
    std::string content_type,
    std::string_view content)
    : m_owner(owner)
    , m_name(name)
    , m_filename(filename)
    , m_content_type(content_type)
    , m_content(content)
{
}
