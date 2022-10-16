#include "torrentsget.hpp"

using porla::Methods::TorrentsGet;

TorrentsGet::TorrentsGet(porla::ISession& session)
    : m_session(session)
{
}

void TorrentsGet::Invoke(const TorrentsGetReq& req, WriteCb<TorrentsGetRes> cb)
{
    cb({});
}
