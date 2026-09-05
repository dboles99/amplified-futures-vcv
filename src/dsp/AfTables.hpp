#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
//
// Named tuning tables. Data, not behaviour.
//
// Every table carries a provenance string, because two of them are modern
// reconstructions of a source that gives COUNTS, NOT RATIOS, and shipping them
// as fact would be wrong. See section 3.3 of the pitch spec.

#include "AfTuning.hpp"

namespace af::tuning {

enum class TableId {
    Equal12,
    HarmonicSeries,
    ChathamNoWave,
    Shruti5Limit,
    ShrutiEqual22,
};

struct Table {
    const char* name;
    const char* provenance;
    const float* cents;
    int count;
};

namespace detail {

// File scope, not constexpr struct members: constexpr member arrays that are
// odr-used hit C++11 ODR problems with this toolchain. House rule.
static const float kEqual12[12] = {
    0.f, 100.f, 200.f, 300.f, 400.f, 500.f,
    600.f, 700.f, 800.f, 900.f, 1000.f, 1100.f
};

// Partials 1..16. Deliberately stops well below the point where spacing
// collapses (53c at n=32); beyond that it is a density control, not a scale.
static const float kHarmonic[16] = {
    0.f,       1200.f,    1901.955f, 2400.f,
    2786.314f, 3101.955f, 3368.826f, 3600.f,
    3803.910f, 3986.314f, 4151.318f, 4301.955f,
    4440.528f, 4568.826f, 4688.269f, 4800.f
};

// Rhys Chatham: dropped D, with the B string tuned to the 7th partial of the
// low D — "a very flat C". D A D A (7/4) D.
static const float kChatham[6] = {
    0.f, 701.955f, 1200.f, 1901.955f, 2400.f + 968.826f, 3600.f
};

// 5-limit reconstruction of the 22 shrutis. RATIOS ARE NOT ATTESTED: Bharata's
// Natyashastra gives counts (4-3-2-4-4-3-2 across the seven svaras) and
// demonstrates the division with two vinas, but no frequency ratios. This is
// the widely circulated modern reconstruction and reconstructions disagree.
// Derived as 1200*log2(ratio) for the standard ratio list; the sequence sums
// to exactly 1200 cents.
static const float kShruti5[22] = {
    0.f,       90.225f,   111.731f,  182.404f,  203.910f,
    294.135f,  315.641f,  386.314f,  407.820f,  498.045f,
    519.551f,  590.224f,  611.730f,  701.955f,  792.180f,
    813.686f,  884.359f,  905.865f,  996.090f,  1017.596f,
    1088.269f, 1109.775f
};

// The equal-shruti reading: 22-EDO, 1200/22 = 54.5454... cents per step. Some
// readings of the Vedic material hold Bharata's shrutis were equal.
static const float kShrutiEq[22] = {
    0.f,        54.5455f,   109.0909f,  163.6364f,  218.1818f,
    272.7273f,  327.2727f,  381.8182f,  436.3636f,  490.9091f,
    545.4545f,  600.f,      654.5455f,  709.0909f,  763.6364f,
    818.1818f,  872.7273f,  927.2727f,  981.8182f,  1036.3636f,
    1090.9091f, 1145.4545f
};

} // namespace detail

[[nodiscard]] inline const Table& table(TableId id) noexcept
{
    static const Table kTables[] = {
        { "12-TET", "equal temperament",
          detail::kEqual12, 12 },
        { "Harmonic series", "partials 1-16, 1200*log2(n)",
          detail::kHarmonic, 16 },
        { "Chatham", "Rhys Chatham, B string to the 7th partial (7/4)",
          detail::kChatham, 6 },
        { "Shruti (5-limit)",
          "5-limit RECONSTRUCTION - ratios are not attested in Bharata",
          detail::kShruti5, 22 },
        { "Shruti (equal)", "22-EDO READING - equal shrutis are disputed, not attested in Bharata",
          detail::kShrutiEq, 22 },
    };
    return kTables[static_cast<int>(id)];
}

} // namespace af::tuning
