#pragma once

#include <unordered_set>

#include <boost/asio/io_context.hpp>
#include <boost/signals2.hpp>
#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "keyvalueset_reqres.hpp"

namespace porla::Rpc::Methods::Kv
{
    class KeyValueSet : public TypedMethod<KeyValueSetReq, KeyValueSetRes>
    {
    public:
        explicit KeyValueSet(
            boost::asio::io_context& io,
            sqlite3* db,
            boost::signals2::signal<void(const std::unordered_set<std::string>&)>& kv_updated);

    protected:
        void Execute(const KeyValueSetReq& req, ResponseWriterHandle cb) override;

    private:
        boost::asio::io_context& m_io;
        sqlite3* m_db;
        boost::signals2::signal<void(const std::unordered_set<std::string>&)>& m_kv_updated;
    };
}
