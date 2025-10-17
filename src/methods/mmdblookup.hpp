#pragma once

#include <filesystem>
#include <memory>
#include <optional>

#include "method.hpp"
#include "mmdblookup_reqres.hpp"

namespace porla
{
    class Mmdb;
}

namespace porla::Methods
{
    class MmdbLookup : public Method<MmdbLookupReq, MmdbLookupRes>
    {
    public:
        explicit MmdbLookup(std::optional<std::filesystem::path> file);

    protected:
        void Invoke(const MmdbLookupReq& req, WriteCb<MmdbLookupRes> cb) override;

    private:
        struct State;
        std::shared_ptr<State> m_state;
    };
}
