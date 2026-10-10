#pragma once

#include <chrono>
#include <memory>
#include <typeindex>
#include <vector>

#include <boost/asio/io_context.hpp>

namespace porla
{
    class Job;
    class Session;

    // manages jobs for a single session
    // all jobs run on the io thread
    class Scheduler
    {
    public:
        Scheduler(boost::asio::io_context& io, Session& session);
        ~Scheduler();

        Scheduler(const Scheduler&)            = delete;
        Scheduler& operator=(const Scheduler&) = delete;

        // runs a job on an interval
        void Every(std::chrono::milliseconds interval, std::unique_ptr<Job> job);

        // starts a job immediately and keeps it running as long
        // as it returns Again()
        void Start(std::unique_ptr<Job> job);

        // registers a job but does not start it
        // requires manual trigger
        void Register(std::unique_ptr<Job> job);

        // trigger a manual job
        template<typename T>
        void Trigger() { Trigger(std::type_index(typeid(T))); }

        // suspends all manual jobs until resume is called
        void Suspend();

        // resume all jobs that where suspended
        void Resume();

        // stops all jobs
        void Stop();

    private:
        enum class Kind { Periodic, Once, Manual };

        struct Entry;
        struct State;

        void Trigger(std::type_index type);

        static std::shared_ptr<Entry> Add(
            State& shared,
            Kind kind,
            std::chrono::milliseconds interval,
            std::unique_ptr<Job> job);

        static void Schedule(
            const std::shared_ptr<State>& shared,
            const std::shared_ptr<Entry>& entry,
            std::chrono::milliseconds delay);

        static void Run(
            const std::shared_ptr<State>& shared,
            const std::shared_ptr<Entry>& entry);

        std::shared_ptr<State> m_state;
    };
}
