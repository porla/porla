#include "torrentsremove.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsRemove;
using porla::Methods::TorrentsRemoveReq;
using porla::Methods::TorrentsRemoveRes;

TorrentsRemove::TorrentsRemove(porla::ISession &session)
    : m_session(session)
{
}

void TorrentsRemove::Invoke(const TorrentsRemoveReq &req, WriteCb<TorrentsRemoveRes> cb)
{
    for (auto const& hash : req.info_hashes)
    {
        m_session.Remove(hash, req.remove_data);
    }

    cb(TorrentsRemoveRes{});
}
