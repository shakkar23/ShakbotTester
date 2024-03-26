#include <cstdint>
#include <type_traits>
// if either clang/gcc
#ifdef __GNUC__
#ifndef __clang__
#ifdef __BMI2__
// gcc
#include <immintrin.h>
#endif
#elif
// clang
#include <immintrin.h>
#endif
#endif

// this is not specific to any architecture or integer size
// but its fine cause we should only be using this on u32s or u64s

template <typename T>
constexpr T pext(const T SRC, const T MASK) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
#ifdef __GNUC__
#ifdef __BMI2__
    if (!std::is_constant_evaluated() && std::is_same<T, uint64_t>::value) {
        return _pext_u64(SRC, MASK);
    } else if (!std::is_constant_evaluated() && std::is_same<T, uint32_t>::value) {
        return _pext_u32(SRC, MASK);
    }
#endif
#endif
    T DEST = 0;
    int m = 0, k = 0;
    do {
        if ((MASK >> m) & 1) {
            DEST |= (SRC & (1 << m)) >> (m - k);
            k = k + 1;
        }
        m = m + 1;
    } while (m < sizeof(T) * 8);

    return DEST;
}
