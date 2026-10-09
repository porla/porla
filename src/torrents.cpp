#include "torrents.hpp"

#include <vector>

#include <boost/log/trivial.hpp>

#include "data/models/addtorrentparams.hpp"
#include "data/models/sessions.hpp"
#include "events.hpp"
#include "presets.hpp"
#include "sessions/session.hpp"
#include "sessions/sessions.hpp"
#include "sessions/torrentevent.hpp"
#include "torrentclientdata.hpp"

using porla::Torrents;
using porla::TorrentsAddOptions;
using porla::TorrentsAddResult;

using Error = TorrentsAddResult::Error;

namespace
{
    static void ApplyPreset(lt::add_torrent_params& p, const porla::Presets::Preset& preset)
    {
        if (preset.download_limit.has_value())  p.download_limit  = preset.download_limit.value();
        if (preset.max_connections.has_value()) p.max_connections = preset.max_connections.value();
        if (preset.max_uploads.has_value())     p.max_uploads     = preset.max_uploads.value();
        if (preset.save_path.has_value())       p.save_path       = preset.save_path.value();
        if (preset.upload_limit.has_value())    p.upload_limit    = preset.upload_limit.value();

        if (preset.storage_mode == "allocate")  p.storage_mode    = lt::storage_mode_allocate;
        if (preset.storage_mode == "sparse")    p.storage_mode    = lt::storage_mode_sparse;

        // Apply flags (if any)
        if (preset.flags.has_value() && preset.flags_mask.has_value())
        {
            const auto flags = preset.flags.value();
            const auto mask  = preset.flags_mask.value();

            p.flags = (p.flags & ~mask) | (flags & mask);
        }

        // Set our custom client data
        if (preset.category.has_value())
            p.userdata.get<porla::TorrentClientData>()->category = preset.category.value();

        if (!preset.tags.empty())
            p.userdata.get<porla::TorrentClientData>()->tags = preset.tags;
    }

    bool IsValidClientData(const porla::TorrentClientData& data)
    {
        try
        {
            nlohmann::json{
                { "category", data.category.value_or("") },
                { "metadata", data.metadata },
                { "tags", data.tags }
            }.dump();

            return true;
        }
        catch (const nlohmann::json::type_error&)
        {
            return false;
        }
    }
}

Torrents::Torrents(const TorrentsOptions& options)
    : m_options(options)
{
}

TorrentsAddResult Torrents::Prepare(lt::add_torrent_params& params, const TorrentsAddOptions& options)
{
    // Which session should we add this torrent to?
    // - If we have a session_id, use that
    // - If we have a preset_id, and that preset has a session_id, use that
    // - If there is a default preset, and that preset has a session_id, use that
    // - If nothing, use the default

    const auto default_preset = m_options.presets.GetDefault();

    std::optional<Presets::Preset> preset;

    if (options.preset_id.has_value())   preset = m_options.presets.Get(options.preset_id.value());
    else if (options.preset.has_value()) preset = m_options.presets.GetByName(options.preset.value());
    else                                 preset = default_preset;


    const std::optional<int> session_id = options.session_id.has_value()
        ? options.session_id
        : preset.has_value() && preset->session_id.has_value()
            ? preset->session_id
            : default_preset.has_value() && default_preset->session_id.has_value()
                ? default_preset->session_id
                : std::nullopt;

    const auto session_record = session_id.has_value()
        ? Data::Models::Sessions::GetById(m_options.db, session_id.value())
        : Data::Models::Sessions::GetDefault(m_options.db);

    if (!session_record)
    {
        return {
            .error      = Error::SessionNotFound,
            .what       = "Session not found",
            .session_id = session_id.value_or(-1)
        };
    }

    const auto session = m_options.sessions.Get(session_record->id);

    if (!session)
    {
        return {
            .error      = Error::SessionNotLoaded,
            .what       = "Session not loaded",
            .session_id = session_record->id
        };
    }

    const auto* existing_client_data = params.userdata.get<TorrentClientData>();

    auto* client_data = new TorrentClientData(
        existing_client_data != nullptr
            ? *existing_client_data
            : TorrentClientData{});

    client_data->session = session;

    params.userdata = lt::client_data_t(client_data);

    if (default_preset.has_value())
    {
        BOOST_LOG_TRIVIAL(info) << "Applying default preset";
        ApplyPreset(params, default_preset.value());
    }

    // Apply the user-specified preset unless it is also the default preset, which has
    // already been applied above.
    if (preset.has_value() && (!default_preset.has_value() || preset->id != default_preset->id))
    {
        BOOST_LOG_TRIVIAL(info) << "Applying preset " << preset->name;
        ApplyPreset(params, preset.value());
    }

    return { .session_id = session->Id() };
}

TorrentsAddResult Torrents::Add(lt::add_torrent_params params)
{
    auto* client_data = params.userdata.get<TorrentClientData>();
    auto  session     = client_data != nullptr ? client_data->session.lock() : nullptr;

    if (session == nullptr)
    {
        delete client_data;
        return { .error = Error::SessionNotLoaded, .what = "Session not loaded" };
    }

    const auto info_hash = params.ti
        ? params.ti->info_hashes()
        : params.info_hashes;

    TorrentsAddResult result{
        .session_id = session->Id(),
        .info_hash  = info_hash
    };

    if (info_hash == lt::info_hash_t())
    {
        result.error = Error::MissingInfoHash;
        result.what  = "Failed to get info hash from params";
    }
    else if (session->Torrents().contains(info_hash)
        || session->Libtorrent().find_torrent(info_hash.get_best()).is_valid())
    {
        result.error = Error::AlreadyInSession;
        result.what  = "Torrent already in session";
    }
    else if (!IsValidClientData(*client_data))
    {
        result.error = Error::InvalidData;
        result.what  = "Category, tags and metadata must be valid UTF-8";
    }
    else if (params.save_path.empty())
    {
        result.error = Error::MissingSavePath;
        result.what  = "'save_path' missing";
    }
    else
    {
        try
        {
            session->Libtorrent().async_add_torrent(std::move(params));
            return result;
        }
        catch (const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to add torrent to session: " << e.what();

            result.error = Error::Failed;
            result.what  = e.what();
        }
    }

    delete client_data;

    return result;
}

bool Torrents::UpdateClientData(const lt::torrent_handle& th, const std::function<void(TorrentClientData&)>& change)
{
    auto* client_data = th.is_valid() ? th.userdata().get<TorrentClientData>() : nullptr;
    auto  session     = client_data != nullptr ? client_data->session.lock() : nullptr;

    if (session == nullptr)
    {
        return false;
    }

    TorrentClientData updated = *client_data;

    change(updated);

    std::vector<std::string> fields;

    if (updated.category != client_data->category) fields.emplace_back("category");
    if (updated.metadata != client_data->metadata) fields.emplace_back("metadata");
    if (updated.tags     != client_data->tags)     fields.emplace_back("tags");

    if (fields.empty())
    {
        return true;
    }

    const auto info_hash = th.info_hashes();

    Data::Models::AddTorrentParams::UpdateClientData(
        m_options.db,
        session->Id(),
        info_hash,
        updated);

    *client_data = std::move(updated);

    if (m_options.events.HasSubscribers("torrent.userdata_updated"))
    {
        TorrentEvent event("torrent.userdata_updated", {{ "fields", fields }});
        event.session_id     = session->Id();
        event.session        = session;
        event.torrent_handle = th;
        event.info_hash      = info_hash;

        m_options.events.Publish(std::move(event));
    }

    return true;
}
