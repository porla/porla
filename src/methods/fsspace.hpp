#pragma once

#include "../config.hpp"
#include "method.hpp"
#include "fsspace_reqres.hpp"

namespace porla::Methods
{
    template <bool SSL> class FsSpace : public Method<FsSpaceReq, FsSpaceRes, SSL>
    {
    public:
        explicit FsSpace();

    protected:
        void Invoke(const FsSpaceReq& req, WriteCb<FsSpaceRes, SSL> cb) override;
    };
}
