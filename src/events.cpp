#include "events.hpp"

using porla::Events;

Events::Events(boost::asio::io_context& io)
    : m_io_context(io)
{
}

boost::signals2::connection Events::On(std::string_view name, const Signal::slot_type& subscriber)
{
    auto it = m_signals.find(name);

    if (it == m_signals.end())
    {
        it = m_signals.emplace(std::string(name), std::make_unique<Signal>()).first;
    }

    return it->second->connect(subscriber);
}

bool Events::HasSubscribers(std::string_view name) const
{
    const auto it = m_signals.find(name);
    return it != m_signals.end() && !it->second->empty();
}

void Events::Deliver(const Event& event)
{
    if (const auto it = m_signals.find(event.name); it != m_signals.end())
    {
        (*it->second)(event);
    }
}
