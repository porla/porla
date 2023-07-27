#include "sessions.hpp"

#include <filesystem>
#include <fstream>

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>

namespace fs = std::filesystem;

using porla::Sessions;
using porla::SessionsLoadOptions;
using porla::SessionsOptions;

static lt::session_params ReadSessionParams(const fs::path& file)
{
    if (fs::exists(file))
    {
        std::ifstream session_params_file(file, std::ios::binary);

        // Get the params file size
        session_params_file.seekg(0, std::ios_base::end);
        const std::streamsize session_params_size = session_params_file.tellg();
        session_params_file.seekg(0, std::ios_base::beg);

        BOOST_LOG_TRIVIAL(info) << "Reading session params (" << session_params_size << " bytes)";

        // Create a buffer to hold the contents of the session params file
        std::vector<char> session_params_buffer;
        session_params_buffer.resize(session_params_size);

        // Actually read the file
        session_params_file.read(session_params_buffer.data(), session_params_size);

        // Only load the DHT state from the session params. Settings are stored elsewhere.
        return lt::read_session_params(session_params_buffer, lt::session::save_dht_state);
    }

    return {};
}

Sessions::Sessions(const SessionsOptions &options)
    : m_options(options)
{
}

std::shared_ptr<Sessions::SessionState> Sessions::Default()
{
    return m_sessions.at("default");
}

std::shared_ptr<Sessions::SessionState> Sessions::Get(const std::string& name)
{
    return m_sessions.at(name);
}

void Sessions::Load(const SessionsLoadOptions& options)
{
    lt::session_params params = ReadSessionParams(options.session_params_file);
    params.settings = options.settings;

    auto state = std::make_shared<SessionState>();
    state->session = std::make_shared<lt::session>(std::move(params));
    state->session->add_extension(&lt::create_ut_metadata_plugin);
    state->session->add_extension(&lt::create_ut_pex_plugin);
    state->session->add_extension(&lt::create_smart_ban_plugin);
    state->torrents = {};

    state->session->set_alert_notify(
        [this, state]()
        {
            boost::asio::post(m_options.io, [this, state] { ReadAlerts(state); });
        });
}

void Sessions::ReadAlerts(const std::shared_ptr<SessionState>& state)
{
    std::vector<lt::alert*> alerts;
    state->session->pop_alerts(&alerts);

    for (auto const alert : alerts)
    {
        BOOST_LOG_TRIVIAL(trace) << "session[" << state->name << "] " << alert->what() << ": " << alert->message();
    }
}
