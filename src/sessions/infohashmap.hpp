#pragma once

#include <map>
#include <optional>
#include <utility>

#include <libtorrent/info_hash.hpp>

namespace porla
{
    // every insert, erase and re-key must go through here, or find_by_half drifts from the keys
    template<typename T>
    class InfoHashMap
    {
    public:
        using Map            = std::map<lt::info_hash_t, T>;
        using iterator       = typename Map::iterator;
        using const_iterator = typename Map::const_iterator;

        iterator       begin()       { return m_items.begin(); }
        iterator       end()         { return m_items.end(); }
        const_iterator begin() const { return m_items.begin(); }
        const_iterator end()   const { return m_items.end(); }

        bool        empty() const { return m_items.empty(); }
        std::size_t size()  const { return m_items.size(); }

        bool           contains(const lt::info_hash_t& hash) const { return m_items.contains(hash); }
        iterator       find(const lt::info_hash_t& hash)           { return m_items.find(hash); }
        const_iterator find(const lt::info_hash_t& hash)     const { return m_items.find(hash); }
        iterator       upper_bound(const lt::info_hash_t& hash)    { return m_items.upper_bound(hash); }

        // also matches a magnet still keyed by one half when given the full hybrid hash
        iterator find_by_half(const lt::info_hash_t& hash)
        {
            const auto key = KeyByHalf(hash);
            return key.has_value() ? m_items.find(*key) : m_items.end();
        }

        const_iterator find_by_half(const lt::info_hash_t& hash) const
        {
            const auto key = KeyByHalf(hash);
            return key.has_value() ? m_items.find(*key) : m_items.end();
        }

        std::pair<iterator, bool> try_emplace(const lt::info_hash_t& hash)
        {
            auto result = m_items.try_emplace(hash);

            if (result.second)
            {
                AddHalves(hash);
            }

            return result;
        }

        void erase(iterator it)
        {
            RemoveHalves(it->first);
            m_items.erase(it);
        }

        // moves the item at `prev` to `curr`; end() when there is none, or when `curr` is taken (the item at
        // `prev` is dropped then, as the map's own extract/insert would)
        iterator rekey(const lt::info_hash_t& prev, const lt::info_hash_t& curr)
        {
            auto node = m_items.extract(prev);

            if (node.empty())
            {
                return m_items.end();
            }

            RemoveHalves(prev);

            node.key() = curr;

            auto result = m_items.insert(std::move(node));

            if (!result.inserted)
            {
                return m_items.end();
            }

            AddHalves(curr);

            return result.position;
        }

    private:
        static bool Compatible(const lt::info_hash_t& a, const lt::info_hash_t& b)
        {
            return (!a.has_v1() || !b.has_v1() || a.v1 == b.v1)
                && (!a.has_v2() || !b.has_v2() || a.v2 == b.v2);
        }

        std::optional<lt::info_hash_t> KeyByHalf(const lt::info_hash_t& hash) const
        {
            if (hash.has_v1())
            {
                if (const auto it = m_v1.find(hash.v1); it != m_v1.end() && Compatible(hash, it->second))
                {
                    return it->second;
                }
            }

            if (hash.has_v2())
            {
                if (const auto it = m_v2.find(hash.v2); it != m_v2.end() && Compatible(hash, it->second))
                {
                    return it->second;
                }
            }

            return std::nullopt;
        }

        // two items sharing a half is pathological; the first one keeps it
        void AddHalves(const lt::info_hash_t& key)
        {
            if (key.has_v1()) { m_v1.try_emplace(key.v1, key); }
            if (key.has_v2()) { m_v2.try_emplace(key.v2, key); }
        }

        void RemoveHalves(const lt::info_hash_t& key)
        {
            if (const auto it = m_v1.find(key.v1); key.has_v1() && it != m_v1.end() && it->second == key) { m_v1.erase(it); }
            if (const auto it = m_v2.find(key.v2); key.has_v2() && it != m_v2.end() && it->second == key) { m_v2.erase(it); }
        }

        Map                                        m_items;
        std::map<lt::sha1_hash, lt::info_hash_t>   m_v1;
        std::map<lt::sha256_hash, lt::info_hash_t> m_v2;
    };
}
