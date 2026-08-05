#include "plugins.hpp"

using porla::Data::Models::Plugins;

std::optional<Plugins::Plugin> Plugins::GetById(sqlite3* db, int id)
{
    return std::nullopt;
}

std::vector<Plugins::Plugin> Plugins::List(sqlite3* db)
{
    return {};
}

void Plugins::Update(sqlite3* db, int id, std::optional<std::string> config, const std::map<std::string, nlohmann::json>& metadata)
{
}
