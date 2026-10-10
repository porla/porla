#pragma once

#include "job.hpp"
#include "../session.hpp"

namespace porla::Jobs
{
    class ReconcileTorrents : public Job
    {
    public:
        std::string_view Name() const override { return "reconcile-torrents"; }

        Result Run(Session& session) override
        {
            session.ReconcileTorrents();
            return Done();
        }
    };
}
