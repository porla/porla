#pragma once

#include <array>
#include <functional>

#include <libtorrent/alert_types.hpp>

namespace porla
{
    class AlertDispatcher
    {
    public:
        template<typename Alert>
        void On(std::function<void(const Alert*)> handler)
        {
            m_handlers[Alert::alert_type] = [h = std::move(handler)](const lt::alert* alert)
            {
                h(static_cast<const Alert*>(alert));
            };
        }

        bool Dispatch(const lt::alert* alert) const
        {
            const auto& handler = m_handlers[alert->type()];

            if (!handler)
            {
                return false;
            }

            handler(alert);

            return true;
        }

    private:
        std::array<std::function<void(const lt::alert*)>, lt::num_alert_types> m_handlers;
    };
}
