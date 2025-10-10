#pragma once

#include <sqlite3.h>

#include "method.hpp"
#include "keyvalueset_reqres.hpp"

namespace porla::Methods
{
    class KeyValueSet : public Method<KeyValueSetReq, KeyValueSetRes>
    {
    public:
        explicit KeyValueSet(sqlite3* db);

    protected:
        void Invoke(const KeyValueSetReq& req, WriteCb<KeyValueSetRes> cb) override;

    private:
        sqlite3* m_db;
    };
}
