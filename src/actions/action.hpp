#pragma once

#include <memory>
#include <optional>

#include <libtorrent/info_hash.hpp>
#include <toml++/toml.h>

namespace porla::Actions
{
    struct ActionCallback;

    class Action
    {
    public:
        virtual ~Action() = default;

        virtual void Invoke(const libtorrent::info_hash_t& hash, const toml::array& args, const std::shared_ptr<ActionCallback>& callback) = 0;
    };
}
