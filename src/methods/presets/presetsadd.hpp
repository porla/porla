#pragma once

#include "../../config.hpp"
#include "../method.hpp"
#include "presetsadd_reqres.hpp"

namespace porla::Methods::Presets
{
    class PresetsAdd : public Method<PresetsAddReq, PresetsAddRes>
    {
    public:
        explicit PresetsAdd(sqlite3* db);

    protected:
        void Invoke(const PresetsAddReq& req, WriteCb<PresetsAddRes> cb) override;

    private:
        sqlite3* m_db;
    };
}
