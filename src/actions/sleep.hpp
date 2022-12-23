#pragma once

#include <boost/asio.hpp>

#include "action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Actions
{
    class Sleep : public Action
    {
    public:
        explicit Sleep(boost::asio::io_context& io);

        void Invoke(const libtorrent::info_hash_t& hash, const std::vector<std::string>& args, const std::shared_ptr<ActionCallback>& callback) override;

    private:
        boost::asio::io_context& m_io;
    };
}
