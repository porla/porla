#pragma once

#include "action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Actions
{
    class Log : public Action
    {
    public:
        explicit Log(ISession& session);

        void Invoke(const libtorrent::info_hash_t& hash, const toml::array& args, const std::shared_ptr<ActionCallback>& callback) override;

    private:
        ISession& m_session;
    };
}
