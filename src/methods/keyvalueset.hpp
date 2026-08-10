#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/signals2.hpp>
#include <sqlite3.h>

#include "method.hpp"
#include "keyvalueset_reqres.hpp"

namespace porla::Methods
{
    class KeyValueSet : public Method<KeyValueSetReq, KeyValueSetRes>
    {
    public:
        explicit KeyValueSet(
            boost::asio::io_context& io,
            sqlite3* db,
            boost::signals2::signal<void(const std::unordered_set<std::string>&)>& kv_updated);

    protected:
        void Invoke(const KeyValueSetReq& req, WriteCb<KeyValueSetRes> cb) override;

    private:
        boost::asio::io_context& m_io;
        sqlite3* m_db;
        boost::signals2::signal<void(const std::unordered_set<std::string>&)>& m_kv_updated;
    };
}
