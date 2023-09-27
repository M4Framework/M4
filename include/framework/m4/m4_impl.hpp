#pragma once
#include "m4.hpp"
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <type_traits>
#include "../framework_utils.hpp"

namespace sketch {
    template <typename META>
    M4<META>::M4(u64 mem_limit, u32 hash_num, u32 seed) {
        // calculate bucket number per level
        TinyCnter tmp_lv0;
        META tmp[4];
        for (u32 i = 1; i < 4; ++i) {
            tmp[i] = createMeta(cap[i], alpha[i], cmtor_cap[i], td_cap[i]);
        }

        u32 bucket_num[LEVELS];
        bucket_num[0] = mem_limit * mem_div[0] / tmp_lv0.memory();
        for (u32 i = 1; i < 4; ++i) {
            bucket_num[i] = mem_limit * mem_div[i] / tmp[i].memory();
        }

        // allocate memory
        lv0.reserve(bucket_num[0]);
        lv1.reserve(bucket_num[1]);
        lv2.reserve(bucket_num[2]);
        lv3.reserve(bucket_num[3]);

        while (bucket_num[0]--) { lv0.emplace_back(); }
        while (bucket_num[1]--) { lv1.push_back(tmp[1]); }
        while (bucket_num[2]--) { lv2.push_back(tmp[2]); }
        while (bucket_num[3]--) { lv3.push_back(tmp[3]); }

        // initialize hash
        rand_u32_generator gen(seed, MAX_PRIME32 - 1);
        for (u32 i = 0; i < LEVELS; ++i) {
            hash[i].reserve(hash_num);
            hashVal[i].reserve(hash_num);
            for (u32 j = 0; j < hash_num; ++j) {
                hash[i].emplace_back(gen());
                hashVal[i].emplace_back(0);
            }
        }
    }

    template <typename META>
    u32 M4<META>::memory() const {
        u32 mem = 0;
        mem += lv0.size() * lv0.front().memory();
        mem += lv1.size() * lv1.front().memory();
        mem += lv2.size() * lv2.front().memory();
        mem += lv3.size() * lv3.front().memory();
        return mem;
    }

    template <typename META>
    void M4<META>::append(u32 id, u32 value) {
        u32 level = calcAppendLevel(id);
        if (level == 0) {
            appendTiny(id, value);
        } else {
            appendMETA(level, id, value);
        }
    }

    template <typename META>
    void M4<META>::appendTiny(u32 id, u32 value) {
        const auto& hv = hashVal[0];
        for (u32 i = 0; i < hv.size(); ++i) {
            u32 pos = hv[i] / 4;
            u32 idx = hv[i] % 4;
            if (!lv0[pos].full(idx)) {
                lv0[pos].append(value, idx);
            }
        }
    }

    template <typename META>
    void M4<META>::appendMETA(u32 level, u32 id, u32 value) {
        auto& vec = getVecMETA(level);
        const auto& hv = hashVal[level];
        for (u32 i = 0; i < hv.size(); ++i) {
            if (!vec[hv[i]].full()) {
                vec[hv[i]].append(value);
            }
        }
    }

    template <typename META>
    Histogram M4<META>::doSUM(u32 id) const {
        u32 level = calcQueryLevel(id);

        if (level == 0) {
            throw std::runtime_error("combine() is not supported in level 0");
        }

        Histogram hist = doMIN(level, id);
        for (u32 i = level - 1; i >= 1; --i) {
            if (hasAnyEmpty(i, id)) {
                continue;
            }
            hist = hist | doMIN(i, id);
        }

        return hist;
    }

    template <typename META>
    u32 M4<META>::quantile(u32 id, f64 nom_rank) const {
        return doSUM(id).quantile(nom_rank);
    }

    template <typename META>
    Histogram M4<META>::doMIN(u32 level, u32 id) const {
        const auto& vec = getVecMETA(level);
        const auto& hv = hashVal[level];
        
#ifdef TEST_DD
        DDSketch res = vec[hv[0]];

        auto& cnters = res.counters;

        for (u32 i = 1; i < hv.size(); ++i) {
            const auto& temp = vec[hv[i]];
            for (u32 j = 0; j < cnters.size(); ++j) {
                cnters[j] = std::min(cnters[j], temp.counters[j]);
            }
        }

        return static_cast<Histogram>(res);

#else
        Histogram hist = static_cast<Histogram>(vec[hv[0]]);
        for (u32 i = 1; i < hv.size(); ++i) {
            hist = hist & vec[hv[i]];
        }
        
        return hist;
#endif
    }

    template <typename META>
    void M4<META>::calcHash(u32 id) const {
        // This function lies in hot path.
        // So we endure the verbose code to improve performance.
        const u32 hash_num = hash[0].size();
        u32 mod;
        
        mod = 4 * lv0.size();
        for (u32 i = 0; i < hash_num; ++i) {
            hashVal[0][i] = hash[0][i].run(id) % mod;
        }
        mod = lv1.size();
        for (u32 i = 0; i < hash_num; ++i) {
            hashVal[1][i] = hash[1][i].run(id) % mod;
        }
        mod = lv2.size();
        for (u32 i = 0; i < hash_num; ++i) {
            hashVal[2][i] = hash[2][i].run(id) % mod;
        }
        mod = lv3.size();
        for (u32 i = 0; i < hash_num; ++i) {
            hashVal[3][i] = hash[3][i].run(id) % mod;
        }
    }

    template <typename META>
    bool M4<META>::isAllFull(u32 level, u32 id) const {
        if (level == 0) {
            for (u32 i = 0; i < hashVal[0].size(); ++i) {
                u32 pos = hashVal[0][i] / 4;
                u32 idx = hashVal[0][i] % 4;
                if (!lv0[pos].full(idx)) {
                    return false;
                }
            }
            return true;
        }

        const auto& vec = getVecMETA(level);
        for (u32 i = 0; i < hashVal[level].size(); ++i) {
            if (!vec[hashVal[level][i]].full()) {
                return false;
            }
        }
        return true;
    }

    template <typename META>
    bool M4<META>::hasAnyFull(u32 level, u32 id) const {
        if (level == 0) {
            for (u32 i = 0; i < hashVal[0].size(); ++i) {
                u32 pos = hashVal[0][i] / 4;
                u32 idx = hashVal[0][i] % 4;
                if (lv0[pos].full(idx)) {
                    return true;
                }
            }
            return false;
        }

        const auto& vec = getVecMETA(level);
        for (u32 i = 0; i < hashVal[level].size(); ++i) {
            if (vec[hashVal[level][i]].full()) {
                return true;
            }
        }
        return false;
    }

    template <typename META>
    bool M4<META>::hasAnyEmpty(u32 level, u32 id) const {
        if (level == 0) {
            for (u32 i = 0; i < hashVal[0].size(); ++i) {
                u32 pos = hashVal[0][i] / 4;
                u32 idx = hashVal[0][i] % 4;
                if (lv0[pos].empty(idx)) {
                    return true;
                }
            }
            return false;
        }

        const auto& vec = getVecMETA(level);
        for (u32 i = 0; i < hashVal[level].size(); ++i) {
            if (vec[hashVal[level][i]].empty()) {
                return true;
            }
        }
        return false;
    }

    template <typename META>
    u32 M4<META>::calcAppendLevel(u32 id) const {
        calcHash(id);
        for (u32 i = 0; i < LEVELS; ++i) {
            if (!hasAnyFull(i, id) || hasAnyEmpty(i, id)) {
                return i;
            }
        }

        throw::runtime_error("the whole META DiffSketch is full");
    }

    template <typename META>
    u32 M4<META>::calcQueryLevel(u32 id) const {
        calcHash(id);
        for (u32 i = 0; i < LEVELS; ++i) {
            if (i != 0 && hasAnyEmpty(i, id)) {
                return i - 1;
            }
            if (!hasAnyFull(i, id)) {
                return i;
            }
        }

        throw::runtime_error("the whole META DiffSketch is full");
    }

    template <typename META>
    auto M4<META>::getVecMETA(u32 level) -> vec_meta& {
        switch (level) {
            case 1: return lv1;
            case 2: return lv2;
            case 3: return lv3;
        }

        throw std::runtime_error(
            "M4<META>::getVecMETA(): shouldn't reach here");
    }

    template <typename META>
    auto M4<META>::getVecMETA(u32 level) const -> const vec_meta& {
        switch (level) {
            case 1: return lv1;
            case 2: return lv2;
            case 3: return lv3;
        }

        throw std::runtime_error(
            "M4<META>::getVecMETA(): shouldn't reach here");
    }

    template <typename META>
    FlowType M4<META>::type(u32 id) const {
        u32 level = calcQueryLevel(id);
        switch (level) {
            case 0: return TINY;
            case 1: return MID;
            case 2: return HUGE;
            case 3: return HUGE;
        }

        throw std::runtime_error(
            "M4<META>::getVecMETA(): shouldn't reach here");
    }
}   // namespace sketch
