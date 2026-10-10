#pragma once

#include "job.hpp"

namespace porla::Jobs
{
    // stores session params
    // removes torrents that libtorrent no longer tracks
    // posts resume data for all torrents requiring it
    class SaveState : public Job
    {
    public:
        std::string_view Name() const override { return "save-state"; }

        Result Run(Session& session) override;
    };
}
