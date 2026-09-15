#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "keyvalueget_reqres.hpp"

namespace porla::Rpc::Methods::Kv
{
    class KeyValueGet : public TypedMethod<KeyValueGetReq, KeyValueGetRes>
    {
    public:
        explicit KeyValueGet(sqlite3* db);

    protected:
        void Execute(const KeyValueGetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
