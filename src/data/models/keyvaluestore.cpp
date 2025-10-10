#include "keyvaluestore.hpp"

#include "../statement.hpp"

using porla::Data::Models::KeyValueStore;
using porla::Data::Statement;

nlohmann::json KeyValueStore::Get(sqlite3* db, const std::string& key)
{
    nlohmann::json value;

    auto stmt = Statement::Prepare(db, "SELECT value FROM kvs WHERE key = $1");
    stmt.Bind(1, std::string_view(key));
    stmt.Step(
        [&value](const auto& row)
        {
            value = nlohmann::json::parse(row.GetBuffer(0));
            return SQLITE_OK;
        });

    return value;
}

void KeyValueStore::Set(sqlite3* db, const std::string& key, const nlohmann::json& value)
{
    const std::string encoded_value = value.dump();

    auto stmt = Statement::Prepare(db, "INSERT INTO kvs (key, value, readonly) VALUES ($1, $2, 0) ON CONFLICT (key) DO UPDATE SET value = excluded.value WHERE readonly = 0");
    stmt.Bind(1, std::string_view(key));
    stmt.Bind(2, std::string_view(encoded_value));
    stmt.Execute();
}
