#pragma once
#include "../../common/BOBHash32.h"
#include "../../common/tiny_counter.hpp"
#include "../../common/histogram.hpp"

namespace sketch {
    template <typename META>
    class SketchSingleTest;

    template <typename META>
    class M4 {
        using vec_tiny = std::vector<TinyCnter>;
        using vec_meta = std::vector<META>;

    public:
        /// @brief Constructor.
        /// @param mem_limit Memory limit in bytes.
        /// @param hash_num Number of hash functions per level, by default 2.
        /// @param seed Seed for generating hash functions, by default 0.
        M4(u64 mem_limit, u32 hash_num = 2, u32 seed = 0);

        /// @brief Append a given item into the sketch.
        /// @param id Item ID.
        /// @param value Item value.
        inline void append(u32 id, u32 value);

        /// @brief Estimate the size of a given flow.
        /// @param id Flow ID.
        inline u32 size(u32 id) const;

        /// @brief Return number of bytes the whole sketch uses.
        inline u32 memory() const;

        /// @brief Estimate the quantile value of a given normalized rank.
        /// @param id Item ID.
        /// @param nom_rank Normalized rank.
        inline u32 quantile(u32 id, f64 nom_rank) const;

        /// @brief Return the type of a given flow.
        /// @param id Flow ID.
        inline FlowType type(u32 id) const;


    private:
        // MetaModel metaType; ///< Meta sketch type.
        static constexpr u32 LEVELS = 4;      ///< Number of levels.

        static constexpr u32 cap[4] = {3, UINT8_MAX, UINT16_MAX, UINT32_MAX};
        static constexpr f64 alpha[4] = {0, 0.5, 0.5, 0.3};
        static constexpr u32 cmtor_cap[4] = {0, 2, 2, 4};
        static constexpr u32 td_cap[4] = {0, 4, 8, 16};
        static constexpr f64 mem_div[4] = {0.03, 0.60, 0.35, 0.02};

        vec_tiny lv0;   ///< Level 0.
        vec_meta lv1;   ///< Level 1.
        vec_meta lv2;   ///< Level 2.
        vec_meta lv3;   ///< Level 3.
        std::vector<BOBHash32> hash[LEVELS];    ///< Hash functions.
        mutable vec_u32 hashVal[LEVELS];        ///< Hash values.

        /// @brief Calculate hash values for a given item.
        inline void calcHash(u32 id) const;

        // Level granularity functions.

        /// @brief Append a given item into lv0 (tiny counter level).
        inline void appendTiny(u32 id, u32 value);
        /// @brief Append a given item into lv1, 2, or 3 (dd level).
        inline void appendMETA(u32 level, u32 id, u32 value);

        /// @brief Estimate absolute rank in a given dd level.
        inline u32 rank(u32 level, u32 id, u32 value, bool inclusive) const;

        /// @brief Calculate the appending level of a given flow.
        inline u32 calcAppendLevel(u32 id) const;
        /// @brief Calculate the query level of a given flow.
        inline u32 calcQueryLevel(u32 id) const;

        inline Histogram doMIN(u32 level, u32 id) const;
        inline Histogram doSUM(u32 id) const;

        // Little helper functions.

        /// @brief Get container by level.
        inline vec_meta& getVecMETA(u32 level);
        /// @brief Get container by level.
        inline const vec_meta& getVecMETA(u32 level) const;

        /// @brief Check if buckets of a given flow are all full
        ///        in a given level.
        inline bool isAllFull(u32 level, u32 id) const;

        inline bool hasAnyFull(u32 level, u32 id) const;

        /// @brief Check if any bucket of a given flow is empty
        ///        in a given level.
        inline bool hasAnyEmpty(u32 level, u32 id) const;
    };
}   // namespace sketch

#include "m4_impl.hpp"