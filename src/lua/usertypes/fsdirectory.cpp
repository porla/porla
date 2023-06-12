#include "fsdirectory.hpp"

#include <filesystem>

#include <boost/log/trivial.hpp>

#include "fsdirectoryentry.hpp"

namespace fs = std::filesystem;
using porla::Lua::UserTypes::FsDirectory;
using porla::Lua::UserTypes::FsDirectoryEntry;

void FsDirectory::Register(sol::state &lua)
{
    sol::usertype<FsDirectory> type = lua.new_usertype<FsDirectory>(
        "fs.Directory",
        sol::no_constructor);

    type["exists"] = [](const FsDirectory& self)
    {
        return fs::exists(self.m_args.as<std::string>());
    };

    type["iterate"] = &FsDirectory::Iterate;
}

FsDirectory::FsDirectory(sol::object args)
    : m_args(std::move(args))
{
}

std::vector<FsDirectoryEntry> FsDirectory::Iterate()
{
    const std::string path = m_args.as<std::string>();

    std::vector<FsDirectoryEntry> result;

    for (const auto& file : fs::directory_iterator(path))
    {
        result.emplace_back(FsDirectoryEntry{file});
    }

    return result;
}
