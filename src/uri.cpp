#include "uri.hpp"

#include <sstream>

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

    const auto fromRange = [](const UriTextRangeA &range) -> std::string
    {
        return {range.first, range.afterLast};
    };

    UriPathSegmentStructA *head(uri.pathHead);
    std::stringstream path;

    while (head)
    {
        path << "/" << fromRange(head->text);
        head = head->next;
    }

    std::string port = {uri.portText.first, uri.portText.afterLast};

    out.scheme = {uri.scheme.first, uri.scheme.afterLast};
    out.host   = {uri.hostText.first, uri.hostText.afterLast};
    out.path   = path.str();
    out.port   = port.empty() ? out.scheme == "http" ? 80 : 443 : std::stoi(port);

    return true;
}
