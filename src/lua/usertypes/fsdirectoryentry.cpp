#include "fsdirectoryentry.hpp"

#include <utility>

using porla::Lua::UserTypes::FsDirectoryEntry;

void FsDirectoryEntry::Register(sol::state& lua)
{
    sol::usertype<FsDirectoryEntry> type = lua.new_usertype<FsDirectoryEntry>(
        "fs.DirectoryEntry",
        sol::no_constructor);

    type["extension"] = sol::property(
        [](const FsDirectoryEntry& self)
        {
            return self.m_entry.path().extension().string();
        });

    type["path"] = sol::property(
        [](const FsDirectoryEntry& self)
        {
            return self.m_entry.path().string();
        });
}

FsDirectoryEntry::FsDirectoryEntry(std::filesystem::directory_entry  entry)
    : m_entry(std::move(entry))
{}
