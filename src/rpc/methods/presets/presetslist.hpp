#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "presetslist_reqres.hpp"

namespace porla::Rpc::Methods::Presets
{
    class PresetsList : public TypedMethod<PresetsListReq, PresetsListRes>
    {
    public:
        explicit PresetsList(sqlite3* db);

    protected:
        void Execute(const PresetsListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
