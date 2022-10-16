#include "torrentsremove.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsRemove;
using porla::Methods::TorrentsRemoveReq;
using porla::Methods::TorrentsRemoveRes;

TorrentsRemove::TorrentsRemove(const std::string &path, porla::ISession &session)
    : MethodT(boost::beast::http::verb::post, path)
    , m_session(session)
{
}

void TorrentsRemove::Invoke(const TorrentsRemoveReq &req, WriteCb<TorrentsRemoveRes> cb)
{
    for (auto const& hash : req.info_hashes)
    {
        m_session.Remove(hash, req.remove_data);
    }

    cb({});
}
