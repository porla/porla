#pragma once

#include <chrono>
#include <string_view>

namespace porla
{
    class Session;

    class Job
    {
    public:
        struct Result
        {
            bool again                      = false;
            std::chrono::milliseconds delay = std::chrono::milliseconds(0);
        };

        static Result Done() { return {}; }
        static Result Again(std::chrono::milliseconds delay = std::chrono::milliseconds(0)) { return { true, delay }; };

        virtual ~Job() = default;

        virtual std::string_view Name() const = 0;
        virtual Result Run(Session& session)  = 0;

        // called whenever the scheduler is stopping (session unloads)
        // if the job has promised someone a callback, fulfill that promise now.
        virtual void Stopped(Session& session) {}
    };
}
