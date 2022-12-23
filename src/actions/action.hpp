#pragma once

#include <libtorrent/info_hash.hpp>

namespace porla::Actions
{
    struct ActionCallback;

    class Action
    {
    public:
        virtual ~Action() = default;

        virtual void Invoke(const libtorrent::info_hash_t& hash, const std::vector<std::string>& args, const std::shared_ptr<ActionCallback>& callback) = 0;
    };
}
