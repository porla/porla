#include "uri.hpp"

#include <uriparser/Uri.h>

using porla::Uri;

bool Uri::Parse(const std::string& value, Uri& out)
{
    UriUriA uri = {};

    UriParserStateA state;
    state.uri = &uri;

    if (uriParseUriA(&state, value.c_str()) != URI_SUCCESS)
    {
        return false;
    }

    std::string port = {uri.portText.first, uri.portText.afterLast};

    out.scheme = {uri.scheme.first, uri.scheme.afterLast};
    out.host   = {uri.hostText.first, uri.hostText.afterLast};
    out.port   = port.empty() ? out.scheme == "http" ? 80 : 443 : std::stoi(port);

    return true;
}
