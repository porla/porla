#include "plugins.hpp"

using porla::Data::Models::Plugins;

std::optional<Plugins::Plugin> Plugins::GetById(sqlite3* db, int id)
{
    return std::nullopt;
}

int Plugins::Insert(sqlite3* db, const Plugins::Plugin& plugin)
{
    return -1;
}

std::vector<Plugins::Plugin> Plugins::List(sqlite3* db)
{
    return {};
}

void Plugins::Remove(sqlite3* db, int id)
{
}

void Plugins::Update(sqlite3* db, const Plugin& plugin)
{
}
