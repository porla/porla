#include "potorrentdata.hpp"

#include "pojson.hpp"
#include "posessionhandle.hpp"

#include "../../torrentclientdata.hpp"
#include "../../torrents.hpp"

using porla::Lua::Types::PoTorrentData;
using porla::TorrentClientData;

namespace
{
    bool IsNull(const sol::object& value)
    {
        return value.get_type() == sol::type::lightuserdata
            && value.as<sol::light<char>>().value() == porla::Lua::Types::PoJson::NullSentinel();
    }
}

void PoTorrentData::Register(sol::state& lua)
{
    lua.new_usertype<PoTorrentData>(
        "PoTorrentData",
        sol::no_constructor,
        "category", sol::readonly_property([](const PoTorrentData& d) { return d.ClientData().category; }),
        "has_tag", [](const PoTorrentData& d, const std::string& tag)
        {
            return d.ClientData().tags.contains(tag);
        },
        "metadata", [](const PoTorrentData& d, const std::string& key, sol::this_state ts) -> std::optional<sol::object>
        {
            const auto& cd  = d.ClientData();
            const auto  val = cd.metadata.find(key);

            if (val != cd.metadata.end())
            {
                if (val->second.is_null())
                {
                    return sol::lua_nil;
                }

                return PoJson::ToLua(ts, val->second, 0);
            }

            return std::nullopt;
        },
        "session", sol::property([](const PoTorrentData& ptd) { return std::make_shared<PoSessionHandle>(ptd.ClientData().session); }),
        "tags", [](const PoTorrentData& d) { return sol::as_table(d.ClientData().tags); },
        "update", [](const PoTorrentData& d, const sol::table& changes, sol::this_state ts)
        {
            std::optional<std::optional<std::string>>      category;
            std::optional<std::unordered_set<std::string>> tags;
            std::optional<nlohmann::json>                  metadata;

            if (const sol::object v = changes["category"]; v.valid() && v.get_type() != sol::type::lua_nil)
            {
                if (v.get_type() == sol::type::string) category = v.as<std::string>();
                else if (IsNull(v))                    category = std::optional<std::string>{};
                else throw sol::error("'category' must be a string, or porla.null to clear it");
            }

            if (const sol::object v = changes["tags"]; v.valid() && v.get_type() != sol::type::lua_nil)
            {
                if (v.get_type() != sol::type::table)
                {
                    throw sol::error("'tags' must be a list of strings");
                }

                tags.emplace();

                for (const auto& [ _, tag ] : v.as<sol::table>())
                {
                    if (tag.get_type() != sol::type::string)
                    {
                        throw sol::error("'tags' must be a list of strings");
                    }

                    tags->insert(tag.as<std::string>());
                }
            }

            if (const sol::object v = changes["metadata"]; v.valid() && v.get_type() != sol::type::lua_nil)
            {
                metadata = PoJson::ToJson(ts, v, 0);

                if (!metadata->is_object())
                {
                    throw sol::error("'metadata' must be a table with string keys");
                }
            }

            d.Update([&](TorrentClientData& cd)
            {
                if (category.has_value()) cd.category = std::move(*category);
                if (tags.has_value())     cd.tags     = std::move(*tags);

                if (metadata.has_value())
                {
                    for (auto& [ key, value ] : metadata->items())
                    {
                        if (value.is_null()) cd.metadata.erase(key);
                        else                 cd.metadata[key] = value;
                    }
                }
            });
        }
    );
}

PoTorrentData::PoTorrentData(lt::torrent_handle th, porla::Torrents& torrents)
    : m_th(th)
    , m_torrents(torrents)
{
}

TorrentClientData& PoTorrentData::ClientData() const
{
    auto* data = m_th.userdata().get<TorrentClientData>();

    if (data == nullptr)
    {
        throw sol::error("Torrent is no longer in the session");
    }

    return *data;
}

void PoTorrentData::Update(const std::function<void(TorrentClientData&)>& change) const
{
    if (!m_torrents.UpdateClientData(m_th, change))
    {
        throw sol::error("Torrent is no longer in the session");
    }
}
