#pragma once

#include "../../config.hpp"
#include "../method.hpp"
#include "presetsupdate_reqres.hpp"

namespace porla::Methods::Presets
{
    class PresetsUpdate : public Method<PresetsUpdateReq, PresetsUpdateRes>
    {
    public:
        explicit PresetsUpdate(sqlite3* db);

    protected:
        void Invoke(const PresetsUpdateReq& req, WriteCb<PresetsUpdateRes> cb) override;

    private:
        sqlite3* m_db;
    };
}
