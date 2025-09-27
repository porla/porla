#pragma once

#include "../../config.hpp"
#include "../method.hpp"
#include "presetslist_reqres.hpp"

namespace porla::Methods::Presets
{
    class PresetsList : public Method<PresetsListReq, PresetsListRes>
    {
    public:
        explicit PresetsList(sqlite3* db);

    protected:
        void Invoke(const PresetsListReq& req, WriteCb<PresetsListRes> cb) override;

    private:
        sqlite3* m_db;
    };
}
