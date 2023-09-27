#pragma once
#include "../common/sketch_defs.hpp"
#include "../meta/dd/ddsketch.hpp"
#include "../meta/mreq/mreq_sketch.hpp"
#include "../meta/tdigest/tdigest.hpp"

namespace sketch {
#ifdef TEST_DD
    DDSketch createMeta(u32 cap, f64 alpha, u32, u32) {
        return DDSketch(cap, alpha);
    }
#elif defined(TEST_MREQ)
    mReqSketch createMeta(u32 cap, f64, u32 cmtor_cap, u32) {
        return mReqSketch(cap, cmtor_cap);
    }
#elif defined(TEST_TD)
    TDigest createMeta(u32 cap, f64, u32, u32 delta) {
        return TDigest(cap, delta);
    }
#endif
}   // namespace sketch
