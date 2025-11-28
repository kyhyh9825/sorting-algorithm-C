#if defined(_MSC_VER)
    #include <intrin.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "sorting.h"

typedef int (*CmpFunc)(const void *a_ptr, const void *b_ptr);

static int is_sorted(void *arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr);
static void shuffle(void *arr, size_t num_of_elements, size_t size_of_element);

void bogo_sort(void *arr ,size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{

}

void bogobogo_sort(void *arr ,size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{

}

static int is_sorted(void *arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr)
{

}

static uint64_t xorshift64(void)
{
    static uint64_t x = 245250329236350234;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return x;
}

static inline uint64_t mul128(uint64_t a, uint64_t b, uint64_t *low)
{
#if defined(__SIZEOF_INT128__)
    unsigned __int128 result = (unsigned __int128)a * b;
    *low = (uint64_t)result;
    return (uint64_t)(result >> 64);

#elif defined(__MSC_VER) && defined(_M_X64)
    #pragma intrinsic(_umul128)
    return _umul128(a, b, low);

#else
    uint64_t a_lo = (uint32_t)a;
    uint64_t a_hi = a >> 32;
    uint64_t b_lo = (uint32_t)b;
    uint64_t b_hi = b >> 32;

    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;

    uint64_t p0_hi = p0 >> 32;
    uint64_t mid = p1 + p0_hi;
    uint64_t mid_lo = (uint32_t)mid;
    uint64_t mid_hi = mid >> 32;

    uint64_t mid2 = p2 + mid_lo;
    uint64_t mid2_hi = mid2 >> 32;

    *low = (mid2 << 32) | (uint32_t)p0;
    return p3 + mid_hi + mid2_hi;

#endif
}

static uint64_t random_bounded(uint64_t limit)
{
    uint64_t x = xorshift64();
    uint64_t l;

    uint64_t h = mul128(x, limit, &l);

    if (SORT_UNLIKELY(l < limit))
    {
        uint64_t t = -limit % limit;
        while (l < t)
        {
            x = xorshift64();
            h = mul128(x, limit, &l);
        }
    }
    return h;
}

static void shuffle(void *arr, size_t num_of_elements, size_t size_of_element)
{
    for (size_t i = num_of_elements - 1; i >= 0; i--)
    {
        size_t j = (size_t)random_bounded(i + 1);
        char *a_ptr = (char *)arr + (i * size_of_element);
        char *b_ptr = (char *)arr + (j * size_of_element);
        generic_swap(a_ptr, b_ptr, size_of_element);
    }
}

int main(void)
{

    system("pause");
    return 0;
}