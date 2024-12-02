#include "sysversions.hpp"

#include <boost/version.hpp>
#include <libtorrent/version.hpp>
#include <openssl/opensslv.h>
#include <sqlite3.h>
#include <toml++/toml.hpp>

#include "../buildinfo.hpp"

using porla::Methods::SysVersions;

void SysVersions::Invoke(const json &req, WriteCb<std::map<std::string, std::string>> cb)
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

    cb.Ok({
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
