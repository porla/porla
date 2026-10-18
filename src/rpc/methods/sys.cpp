#include "sys.hpp"

#include <boost/version.hpp>
#include <boost/asio/awaitable.hpp>
#include <curl/curl.h>
#include <libtorrent/version.hpp>
#include <maxminddb.h>
#include <openssl/opensslv.h>
#include <sqlite3.h>
#include <toml++/toml.hpp>
#include <zip.h>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../buildinfo.hpp"
#include "../../data/models/users.hpp"
#include "../../json/utils.hpp"
#include "../../keyvalue.hpp"

namespace porla::Rpc::Methods::Sys
{
    namespace
    {
        boost::asio::awaitable<nlohmann::json> Status(sqlite3* db, const nlohmann::json& req)
        {
            auto any_users = Data::Models::Users::Any(db);

            co_return nlohmann::json({
                {"status", any_users ? "ok" : "setup"}
            });
        }

        boost::asio::awaitable<nlohmann::json> Versions(const nlohmann::json& req)
        {
            std::stringstream boost_version;
            boost_version << BOOST_VERSION / 100000 << "."
                        << BOOST_VERSION / 100 % 1000 << "."
                        << BOOST_VERSION % 100;

            std::stringstream nljson;
            nljson << NLOHMANN_JSON_VERSION_MAJOR << "."
                << NLOHMANN_JSON_VERSION_MINOR << "."
                << NLOHMANN_JSON_VERSION_PATCH;

            std::stringstream toml_version;
            toml_version << TOML_LIB_MAJOR << "."
                << TOML_LIB_MINOR << "."
                << TOML_LIB_PATCH;

            co_return nlohmann::json({
                {"porla", {
                    {"branch", porla::BuildInfo::Branch()},
                    {"commitish", porla::BuildInfo::Commitish()},
                    {"version", porla::BuildInfo::Version()}
                }},
                {"libtorrent", {
                    {"revision", LIBTORRENT_REVISION},
                    {"version", LIBTORRENT_VERSION}
                }},
                {"boost", {
                    {"version", boost_version.str()}
                }},
                {"curl", {
                    {"version", curl_version_info(CURLVERSION_NOW)->version}
                }},
                {"libzip", {
                    {"version", zip_libzip_version()}
                }},
                {"maxminddb", {
                    {"version", MMDB_lib_version()}
                }},
                {"nlohmann_json", {
                    {"version", nljson.str()}
                }},
                {"openssl", {
                    {"release_date", OPENSSL_RELEASE_DATE},
                    {"version", OPENSSL_VERSION_STR},
                    {"version_text", OPENSSL_VERSION_TEXT}
                }},
                {"sqlite", {
                    {"source_id", SQLITE_SOURCE_ID},
                    {"version", SQLITE_VERSION}
                }},
                {"tomlplusplus", {
                    {"version", toml_version.str()}
                }}
            });
        }
    }

    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, sqlite3* db)
    {
        using Kind = Auth::Context::Kind;

        jsonrpc.Register("sys.status",   { Kind::Anon, Kind::User, Kind::Machine }, MakeMethod<nlohmann::json>(io.get_executor(), std::bind_front(Status, db)));
        jsonrpc.Register("sys.versions", {             Kind::User, Kind::Machine }, MakeMethod<nlohmann::json>(io.get_executor(), Versions));
    }
}
