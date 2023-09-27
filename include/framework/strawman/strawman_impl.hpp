#pragma once
#include "strawman.hpp"
#include <type_traits>
#include <algorithm>
#include "../framework_utils.hpp"

namespace sketch {
    template <typename META>
    Strawman<META>::Strawman(u64 mem_limit, u32 seed) {
        dft = createMeta(UINT32_MAX, alpha, cmtor_cap, td_cap);
        u32 bucket_num = mem_limit / (dft.memory() + sizeof(u32)) / HASH_NUM;
        for (u32 i = 0; i < HASH_NUM; ++i) {
            buckets[i] = vector<META>(bucket_num);
            ids[i] = vector<u32>(bucket_num);
            for (u32 j = 0; j < bucket_num; ++j) {
                evict(i, j);
            }
            hash[i].initialize(seed + i);
        }

        dft = createMeta(UINT32_MAX, 0.5, 2, 4);
    }

    template <typename META>
    void Strawman<META>::evict(u32 bucket_id, u32 pos) {
        auto& sketch = buckets[bucket_id][pos];
        sketch = createMeta(UINT32_MAX, alpha, cmtor_cap, td_cap);
        ids[bucket_id][pos] = UINT32_MAX;
    }

    template <typename META>
    void Strawman<META>::append(u32 id, u32 value) {
        min_item = std::min(min_item, value);
        max_item = std::max(max_item, value);
        dft.append(value);

        u32 tmp[HASH_NUM];
        for (u32 i = 0; i < HASH_NUM; ++i) {
            tmp[i] = pos(i, id);
            if (ids[i][tmp[i]] == id) {
                buckets[i][tmp[i]].append(value);
                return;
            }
        }

        for (u32 i = 0; i < HASH_NUM; ++i) {
            if (ids[i][tmp[i]] == UINT32_MAX) {
                ids[i][tmp[i]] = id;
                buckets[i][tmp[i]].append(value);
                return;
            }
        }
    }

    template <typename META>
    u32 Strawman<META>::quantile(u32 id, f64 nom_rank) const {
        for (u32 i = 0; i < HASH_NUM; ++i) {
            u32 tmp = pos(i, id);
            if (ids[i][tmp] == id) {
                return buckets[i][tmp].quantile(nom_rank);
            }
        }

        return dft.quantile(nom_rank);
    }

    template <typename META>
    u32 Strawman<META>::pos(u32 bucket_id, u32 id) const {
        return hash[bucket_id].run(id) % buckets[bucket_id].size();
    }

    template <typename META>
    u32 Strawman<META>::memory() const {
        u32 meta_mem = dft.memory() + sizeof(u32);
        return (HASH_NUM * buckets.size() + 1) * meta_mem;
    }
}   // namespace sketch
