#include "torrents.hpp"

#include <vector>

#include <boost/log/trivial.hpp>

#include "data/models/addtorrentparams.hpp"
#include "data/models/sessions.hpp"
#include "events.hpp"
#include "presets.hpp"
#include "sessions.hpp"
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

    const auto session = session_id.has_value()
        ? Data::Models::Sessions::GetById(m_options.db, session_id.value())
        : Data::Models::Sessions::GetDefault(m_options.db);

    if (!session)
    {
        return {
            .error      = Error::SessionNotFound,
            .what       = "Session not found",
            .session_id = session_id.value_or(-1)
        };
    }

    const auto state = m_options.sessions.Get(session->id);

    if (!state)
    {
        return {
            .error      = Error::SessionNotLoaded,
            .what       = "Session not loaded",
            .session_id = session->id
        };
    }

    const auto* existing_client_data = params.userdata.get<TorrentClientData>();

    auto* client_data = new TorrentClientData(
        existing_client_data != nullptr
            ? *existing_client_data
            : TorrentClientData{});

    client_data->state = state;

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

    return { .session_id = state->id };
}

TorrentsAddResult Torrents::Add(lt::add_torrent_params params)
{
    auto* client_data = params.userdata.get<TorrentClientData>();
    auto  state       = client_data != nullptr ? client_data->state.lock() : nullptr;

    if (state == nullptr)
    {
        delete client_data;
        return { .error = Error::SessionNotLoaded, .what = "Session not loaded" };
    }

    const auto info_hash = params.ti
        ? params.ti->info_hashes()
        : params.info_hashes;

    TorrentsAddResult result{
        .session_id = state->id,
        .info_hash  = info_hash
    };

    if (info_hash == lt::info_hash_t())
    {
        result.error = Error::MissingInfoHash;
        result.what  = "Failed to get info hash from params";
    }
    else if (state->torrents.contains(info_hash))
    {
        result.error = Error::AlreadyInSession;
        result.what  = "Torrent already in session";
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
            state->session->async_add_torrent(std::move(params));
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
    auto  state       = client_data != nullptr ? client_data->state.lock() : nullptr;

    if (state == nullptr)
    {
        return false;
    }

    const TorrentClientData before = *client_data;

    change(*client_data);

    std::vector<std::string> fields;

    if (client_data->category != before.category) fields.emplace_back("category");
    if (client_data->metadata != before.metadata) fields.emplace_back("metadata");
    if (client_data->tags     != before.tags)     fields.emplace_back("tags");

    if (fields.empty())
    {
        return true;
    }

    const auto info_hash = th.info_hashes();

    Data::Models::AddTorrentParams::UpdateClientData(
        m_options.db,
        state->id,
        info_hash,
        *client_data);

    if (m_options.events.HasSubscribers("torrent.userdata_updated"))
    {
        TorrentEvent event("torrent.userdata_updated", {{ "fields", fields }});
        event.session_id     = state->id;
        event.session        = state;
        event.torrent_handle = th;
        event.info_hash      = info_hash;

        m_options.events.Publish(std::move(event));
    }

    return true;
}
