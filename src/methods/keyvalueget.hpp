#pragma once

#include <sqlite3.h>

#include "method.hpp"
#include "keyvalueget_reqres.hpp"

namespace porla::Methods
{
    class KeyValueGet : public Method<KeyValueGetReq, KeyValueGetRes>
    {
    public:
        explicit KeyValueGet(sqlite3* db);

    protected:
        void Invoke(const KeyValueGetReq& req, WriteCb<KeyValueGetRes> cb) override;

    private:
        sqlite3* m_db;
    };
}
