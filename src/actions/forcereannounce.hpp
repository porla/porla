#pragma once

#include <boost/asio.hpp>

#include "action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Actions
{
    class ForceReannounce : public Action
    {
    public:
        explicit ForceReannounce(ISession& session);

        void Invoke(const libtorrent::info_hash_t& hash, const toml::array& args, const std::shared_ptr<ActionCallback>& callback) override;

    private:
        ISession& m_session;
    };
}
