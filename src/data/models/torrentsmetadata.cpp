#include "torrentsmetadata.hpp"

#include <sstream>

#include "../statement.hpp"

using porla::Data::Models::TorrentsMetadata;

namespace lt = libtorrent;
using json = nlohmann::json;
using porla::Data::Statement;

template<typename T>
static std::string ToString(const T &hash)
{
    std::stringstream ss;
    ss << hash;
    return ss.str();
}

std::map<std::string, json> TorrentsMetadata::GetAll(sqlite3* db, const lt::info_hash_t& hash)
{
    return {};
}

void TorrentsMetadata::RemoveAll(sqlite3* db, const libtorrent::info_hash_t& hash)
{
    auto stmt = Statement::Prepare(
        db,
        "DELETE FROM torrentsmetadata\n"
        "WHERE (info_hash_v1 = $1 AND info_hash_v2 IS NULL)\n"
        "   OR (info_hash_v1 IS NULL AND info_hash_v2 = $2)\n"
        "   OR (info_hash_v1 = $1 AND info_hash_v2 = $2);");

    stmt
        .Bind(1, hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind(2, hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Execute();
}

void TorrentsMetadata::Set(sqlite3* db, const lt::info_hash_t& hash, const std::string& key, const json& value)
{
    auto stmt = Statement::Prepare(
        db,
        "REPLACE INTO torrentsmetadata (info_hash_v1, info_hash_v2, key, value) VALUES ($1, $2, $3, $4);");

    stmt.Bind(1, hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind(2, hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind(3, std::string_view(key))
        .Bind(4, std::string_view(value.dump()))
        .Execute();
}
