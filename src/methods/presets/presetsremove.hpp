#pragma once

#include "../../config.hpp"
#include "../method.hpp"
#include "presetsremove_reqres.hpp"

namespace porla::Methods::Presets
{
    class PresetsRemove : public Method<PresetsRemoveReq, PresetsRemoveRes>
    {
    public:
        explicit PresetsRemove(sqlite3* db);

    protected:
        void Invoke(const PresetsRemoveReq& req, WriteCb<PresetsRemoveRes> cb) override;

    private:
        sqlite3* m_db;
    };
}
