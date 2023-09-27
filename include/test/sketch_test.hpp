#pragma once
#include <unordered_set>
#include "../framework/m4/m4.hpp"
#include "../framework/strawman/strawman.hpp"
#include "../common/real_dist.hpp"

namespace sketch {
    template <typename META>
    class SketchSingleTest {
    public:
        /// @brief Constructor.
        /// @param mem_limit Memory limit in bytes.
        /// @param hash_num Number of hash functions per level.
        /// @param seed Seed for generating hash functions.
        /// @param dataset Dataset to be tested.
        SketchSingleTest(u64 mem_limit, u32 hash_num, u32 seed,
                         const vector<FlowItem>& dataset);

        /// @brief Calculate ALE of a given model on a given flow type.
        inline f64 ALE(u32 model, u32 type) const;
        /// @brief Calculate APE of a given model on a given flow type.
        inline f64 APE(u32 model, u32 type) const;
        /// @brief Calculate appending throughput of a given model in Mops.
        inline f64 appendTp(u32 model) const;
        /// @brief Calculate query throughput of a given model in Mops.
        inline f64 queryTp(u32 model) const;

    private:
        M4<META> m4;    ///< M4 model.
        Strawman<META> straw;    ///< Strawman model.
        real_dist real;             ///< Real distribution.

        std::unordered_set<u32> id_list;    ///< List of flow id.

        f64 append_tp[NUM_MODELS];     ///< Appending throughput.

        /// Given percentage, used when calculating ALE and APE.
        static constexpr f64 given_p = 0.5;

        /// @brief Return the type of a given flow.
        /// @param id Flow ID.
        inline FlowType getType(u32 id) const;

        template <typename T>
        inline f64 ALE(const T& sketch, u32 type) const;

        template <typename T>
        inline f64 APE(const T& sketch, u32 type) const;

        template <typename T>
        inline f64 queryTp(const T& sketch) const;

        template <typename T>
        inline f64 FlowALE(const T& sketch, u32 id) const;

        template <typename T>
        inline f64 FlowAPE(const T& sketch, u32 id) const;
    };

    template <typename META>
    class SketchTest {
    public:
        /// @brief Constructor.
        /// @param mem_limit_ Memory limit in bytes.
        /// @param hash_num_ Number of hash functions per level,
        ///                 used only by M4 model.
        /// @param seed_ Seed for generating hash functions.
        /// @param dataset_ Dataset to be tested.
        /// @param repeat_time_ Number of times to repeat the test.
        SketchTest(u64 mem_limit_, u32 hash_num_, u32 seed_,
                  const string& dataset_,
                  u32 repeat_time_);

        /// @brief Run the test.
        void run();

        /// @brief Calculate ALE of a given model.
        f64 ALE(u32 model) const;
        /// @brief Calculate APE of a given model.
        f64 APE(u32 model) const;
        /// @brief Calculate appending throughput of a given model in Mops.
        f64 appendTp(u32 model) const;
        /// @brief Calculate query throughput of a given model in Mops.
        f64 queryTp(u32 model) const;

    private:
        u64 mem_limit;      ///< Memory limit.
        u32 hash_num;       ///< Number of hash functions per level.
        u32 seed;           ///< Seed for generating hash functions.
        string dataset;     ///< Dataset to be tested.
        u32 repeat;         ///< Number of times to repeat the test.

        f64 m_ALE[NUM_MODELS], m_APE[NUM_MODELS];
        f64 m_appendTp[NUM_MODELS], m_queryTp[NUM_MODELS];

        void addMetrics(const SketchSingleTest<META>& test);
        void summarize();
    };
}   // namespace sketch

#include "sketch_test_impl.hpp"
