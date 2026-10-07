#pragma once

#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/signals2.hpp>
#include <nlohmann/json.hpp>

#include "event.hpp"

namespace porla
{
    class Events
    {
    public:
        using Signal = boost::signals2::signal<void(const Event&)>;

        explicit Events(boost::asio::io_context& io);

        Events(const Events&)            = delete;
        Events& operator=(const Events&) = delete;

        boost::signals2::connection On(std::string_view name, const Signal::slot_type& subscriber);

        bool HasSubscribers(std::string_view name) const;

        template<typename T>
        void Publish(T event)
        {
            static_assert(std::is_base_of_v<Event, T>, "events must derive from porla::Event");

            if (!HasSubscribers(event.name))
            {
                return;
            }

            event.id = ++m_last_event_id;

            boost::asio::post(
                m_io_context,
                [this, published = std::make_shared<const T>(std::move(event))]()
                {
                    Deliver(*published);
                });
        }

    private:
        void Deliver(const Event& event);

        boost::asio::io_context& m_io_context;
        std::uint64_t            m_last_event_id = 0;

        // store one signal per event name - never erased.
        std::map<std::string, std::unique_ptr<Signal>, std::less<>> m_signals;
    };
}
