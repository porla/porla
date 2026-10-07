#pragma once

#include <optional>
#include <string>
#include <vector>

#include <boost/signals2.hpp>
#include <sqlite3.h>

#include "data/models/presets.hpp"
#include "presetevent.hpp"

namespace porla
{
    class Events;

    struct PresetsOptions
    {
        sqlite3* db;
        Events&  events;
    };

    class Presets
    {
    public:
        using Preset = Data::Models::Presets::Preset;

        explicit Presets(const PresetsOptions& options);

        Presets(const Presets&)            = delete;
        Presets& operator=(const Presets&) = delete;

        int Add(const std::string& name);
        std::vector<Preset> All() const;
        std::optional<Preset> Get(int id) const;
        std::optional<Preset> GetByName(const std::string& name) const;
        std::optional<Preset> GetDefault() const;
        void Remove(int id);
        void Update(const Preset& preset);

    private:
        void ClearSession(int session_id);
        void PublishUpdated(int id);

        PresetsOptions                     m_options;
        boost::signals2::scoped_connection m_session_removed;
    };
}
