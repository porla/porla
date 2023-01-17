#pragma once

#include "../config.hpp"
#include "method.hpp"
#include "fsspace_reqres.hpp"

namespace porla::Methods
{
    class FsSpace : public Method<FsSpaceReq, FsSpaceRes>
    {
    public:
        explicit FsSpace();

    protected:
        void Invoke(const FsSpaceReq& req, WriteCb<FsSpaceRes> cb) override;
    };
}
