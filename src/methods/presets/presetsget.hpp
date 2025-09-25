#pragma once

#include "../../config.hpp"
#include "../method.hpp"
#include "presetsget_reqres.hpp"

namespace porla::Methods::Presets
{
    class PresetsGet : public Method<PresetsGetReq, PresetsGetRes>
    {
    public:
        explicit PresetsGet(sqlite3* db);

    protected:
        void Invoke(const PresetsGetReq& req, WriteCb<PresetsGetRes> cb) override;

    private:
        sqlite3* m_db;
    };
}
