#include "pohttpserverresponse.hpp"

#include <boost/log/trivial.hpp>
#include <uWebSockets/Multipart.h>

#include "pohttpformfile.hpp"
#include "../pluginstate.hpp"

using porla::Lua::Types::PoHttpServerResponse;

namespace
{
    std::string_view Trim(std::string_view s)
    {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
        while (!s.empty() && (s.back()  == ' ' || s.back()  == '\t')) s.remove_suffix(1);
        return s;
    }

    bool IEquals(std::string_view a, std::string_view b)
    {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
                return false;
        return true;
    }

    int HexVal(char c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    std::string PercentDecode(std::string_view s)
    {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == '%' && i + 2 < s.size())
            {
                int h = HexVal(s[i+1]), l = HexVal(s[i+2]);
                if (h >= 0 && l >= 0) { out.push_back(char(h * 16 + l)); i += 2; continue; }
            }
            out.push_back(s[i]);
        }
        return out;
    }

    // Reads one parameter value: bare token, or "quoted string" with \ escapes.
    // Advances `in` past the value.
    std::string ReadParamValue(std::string_view& in)
    {
        in = Trim(in);
        if (!in.empty() && in.front() == '"')
        {
            in.remove_prefix(1);
            std::string out;
            while (!in.empty() && in.front() != '"')
            {
                if (in.front() == '\\' && in.size() > 1) { in.remove_prefix(1); }
                out.push_back(in.front());
                in.remove_prefix(1);
            }
            if (!in.empty()) { in.remove_prefix(1); } // closing quote
            auto semi = in.find(';');
            in = (semi == std::string_view::npos) ? std::string_view{} : in.substr(semi + 1);
            return out;
        }

        auto semi = in.find(';');
        auto tok  = Trim(in.substr(0, semi));
        in = (semi == std::string_view::npos) ? std::string_view{} : in.substr(semi + 1);
        return std::string{tok};
    }

    struct Disposition
    {
        std::string name;
        std::string filename;
        bool        is_file = false;
    };

    Disposition ParseDisposition(std::string_view value)
    {
        Disposition d;
        bool have_ext_filename = false;

        auto semi = value.find(';');
        if (semi == std::string_view::npos) { return d; }
        value = value.substr(semi + 1);

        while (!value.empty())
        {
            value = Trim(value);
            auto eq = value.find('=');
            if (eq == std::string_view::npos) { break; }

            auto key = Trim(value.substr(0, eq));
            value = value.substr(eq + 1);
            auto val = ReadParamValue(value);

            if (IEquals(key, "name"))
            {
                d.name = val;
            }
            else if (IEquals(key, "filename") && !have_ext_filename)
            {
                d.filename = val;
                d.is_file  = true;
            }
            else if (IEquals(key, "filename*"))
            {
                // RFC 5987: UTF-8''%E2%80%A6  — wins over plain filename
                auto p = val.find("''");
                d.filename = PercentDecode(p == std::string::npos
                                           ? std::string_view{val}
                                           : std::string_view{val}.substr(p + 2));
                d.is_file = true;
                have_ext_filename = true;
            }
        }

        return d;
    }

    std::string PercentDecodePlus(std::string_view s)
    {
        std::string out;
        out.reserve(s.size());

        for (size_t i = 0; i < s.size(); ++i)
        {
            char c = s[i];

            if (c == '+') { out.push_back(' '); continue; }

            if (c == '%' && i + 2 < s.size())
            {
                int h = HexVal(s[i + 1]), l = HexVal(s[i + 2]);
                if (h >= 0 && l >= 0) { out.push_back(char(h * 16 + l)); i += 2; continue; }
            }

            out.push_back(c);
        }

        return out;
    }

    template<class F>
    void ParseUrlEncoded(std::string_view in, F&& emit)
    {
        while (!in.empty())
        {
            auto amp  = in.find('&');
            auto pair = in.substr(0, amp);

            if (!pair.empty())
            {
                auto eq = pair.find('=');

                emit(PercentDecodePlus(pair.substr(0, eq)),
                     eq == std::string_view::npos
                         ? std::string{}
                         : PercentDecodePlus(pair.substr(eq + 1)));
            }

            if (amp == std::string_view::npos) { break; }
            in.remove_prefix(amp + 1);
        }
    }
}

void PoHttpServerResponse::Register(sol::state& lua)
{
    lua.new_usertype<PoHttpServerResponse>(
        "PoHttpServerResponse",
        sol::no_constructor,
        "finish", sol::overload(
            sol::resolve<void()>(&PoHttpServerResponse::Finish),
            sol::resolve<void(const std::string&)>(&PoHttpServerResponse::Finish)
        ),
        "write", &PoHttpServerResponse::Write,
        "writeHeader", &PoHttpServerResponse::WriteHeader,
        "writeStatus", &PoHttpServerResponse::WriteStatus);
}

PoHttpServerResponse::PoHttpServerResponse(std::shared_ptr<LuaState> lua_state, uWS::HttpRequest* req, uWS::HttpResponse<false>* response, std::size_t callback_id)
    : m_body(std::make_shared<std::string>())
    , m_callback_id(callback_id)
    , m_is_aborted(false)
    , m_response(response)
    , m_state(lua_state)
    , m_request(lua_state->lua.create_table())
{
    sol::table headers = lua_state->lua.create_table();

    for (auto [key, value] : *req)
    {
        headers[std::string{key}] = std::string{value};
    }

    m_request["method"]  = std::string{req->getMethod()};
    m_request["path"]    = std::string{req->getUrl()};
    m_request["query"]   = std::string{req->getQuery()};
    m_request["url"]     = std::string{req->getUrl()};
    m_request["headers"] = headers;

    m_content_type = std::string{req->getHeader("content-type")};
}

void PoHttpServerResponse::Setup()
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    m_response->onAborted([weak = weak_from_this()]()
    {
        auto self = weak.lock();
        if (self == nullptr) { return; }

        self->m_is_aborted = true;

        auto state = self->m_state.lock();
        if (state == nullptr) { return; }

        state->http_responses.erase(self);
    });

    m_response->onDataV2(
        [weak = weak_from_this()](std::string_view data, std::uint64_t len)
        {
            auto self = weak.lock();
            if (self == nullptr) { return; }

            self->OnData(data, len);
        });

    state->http_responses.insert(shared_from_this());
}

void PoHttpServerResponse::BuildForm(LuaState& state)
{
    auto mime = Trim(std::string_view{m_content_type}.substr(0, m_content_type.find(';')));

    const bool is_multipart  = IEquals(mime, "multipart/form-data");
    const bool is_urlencoded = IEquals(mime, "application/x-www-form-urlencoded");

    if (!is_multipart && !is_urlencoded)
    {
        if (m_body) { m_request["body"] = *m_body; }
        return;
    }

    sol::table form   = state.lua.create_table();
    sol::table fields = state.lua.create_table();
    sol::table files  = state.lua.create_table();

    if (is_urlencoded)
    {
        if (m_body)
        {
            ParseUrlEncoded(*m_body, [&](const std::string& k, const std::string& v)
            {
                if (!k.empty()) { fields[k] = v; }
            });
        }

        form["kind"] = "urlencoded";
        form["ok"]   = true;
        form["body"] = m_body ? *m_body : std::string{};
    }
    else
    {
        form["kind"] = "multipart";
        form["ok"]   = ParseMultipart(state, fields, files);
    }

    form["fields"] = fields;
    form["files"]  = files;

    m_request["form"] = form;
}

bool PoHttpServerResponse::ParseMultipart(LuaState& state, sol::table& fields, sol::table& files)
{
    if (!m_body || m_body->empty()) { return false; }

    uWS::MultipartParser mp(m_content_type);
    if (!mp.isValid()) { return false; }

    mp.setBody(*m_body);

    std::pair<std::string_view, std::string_view> headers[16];
    int file_index  = 1;
    int part_count  = 0;

    while (auto part = mp.getNextPart(headers))
    {
        if (++part_count > 256) { return false; }   // cheap DoS guard

        std::string_view disposition;
        std::string      part_ct;

        for (auto& h : headers)
        {
            if (h.first.empty()) { break; }          // terminator
            if (h.first == "content-disposition") { disposition = h.second; }
            else if (h.first == "content-type")    { part_ct = std::string{h.second}; }
        }

        if (disposition.empty()) { continue; }

        auto d = ParseDisposition(disposition);
        if (d.name.empty()) { continue; }

        if (d.is_file)
        {
            files[file_index++] = std::make_shared<PoHttpFormFile>(
                m_body,
                d.name,
                d.filename,
                part_ct.empty() ? "application/octet-stream" : part_ct,
                *part);
        }
        else
        {
            fields[d.name] = std::string{*part};
        }
    }

    return true;
}


void PoHttpServerResponse::Finish()
{
    if (m_is_aborted) { return; }
    m_response->end();
}

void PoHttpServerResponse::Finish(const std::string& data)
{
    if (m_is_aborted) { return; }
    m_response->end(data);
}

void PoHttpServerResponse::OnData(std::string_view data, std::uint64_t len)
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        m_response->close();
        return;
    }

    if (!data.empty())
    {
        m_body->append(data);
    }

    if (len == 0)
    {
        auto self = shared_from_this();
        state->http_responses.erase(self);

        BuildForm(*state);

        boost::asio::post(state->io, [self]()
        {
            if (self->m_is_aborted) { return; }

            auto state = self->m_state.lock();
            if (state == nullptr) { return; }
            state->InvokeCallback(self->m_callback_id, self->m_request, self);
        });
    }
}

void PoHttpServerResponse::Write(const std::string& data)
{
    if (m_is_aborted) { return; }
    m_response->write(data);
}

void PoHttpServerResponse::WriteHeader(const std::string& key, const std::string& value)
{
    if (m_is_aborted) { return; }
    m_response->writeHeader(key, value);
}

void PoHttpServerResponse::WriteStatus(const std::string& status)
{
    if (m_is_aborted) { return; }
    m_response->writeStatus(status);
}
