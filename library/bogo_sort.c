#if defined(_MSC_VER)
    #include <intrin.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "sorting.h"

typedef int (*CmpFunc)(const void *a_ptr, const void *b_ptr);

static inline uint64_t xorshift64(void);
static inline uint64_t mul128(uint64_t a, uint64_t b, uint64_t *low);
static uint64_t random_bounded(uint64_t limit);
static void shuffle(void *arr, size_t num_of_elements, size_t size_of_element);
static int is_sorted(void *arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr);
int is_bogobogo_sorted(void *arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr);

void bogo_sort(void *arr ,size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (num_of_elements <= 1)
    {
        return;
    }
    while (!is_sorted(arr, num_of_elements, size_of_element, cmp_func_ptr))
    {
        shuffle(arr, num_of_elements, size_of_element);
    }
}

static inline uint64_t xorshift64(void)
{
    static uint64_t x = 0;
    if (x == 0)
    {
        x = (uint64_t)time(NULL) ^ 234523452374537324ULL;
    }
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

#elif defined(_MSC_VER) && defined(_M_X64)
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
    for (size_t i = num_of_elements; i-- > 0;)
    {
        size_t j = (size_t)random_bounded(i + 1);
        char *a_ptr = (char *)arr + (i * size_of_element);
        char *b_ptr = (char *)arr + (j * size_of_element);
        generic_swap(a_ptr, b_ptr, size_of_element);
    }
}

static int is_sorted(void *arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr)
{
    for (size_t i = 0; i < num_of_elements - 1; i++)
    {
        void *current = (char *)arr + (i * size_of_element);
        void *next = (char *)current + size_of_element;
        if (cmp_func_ptr(current, next) > 0)
        {
            return 0;
        }
    }
    return 1;
}

int is_bogobogo_sorted(void *arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr)
{
    void *copy = malloc(num_of_elements * size_of_element);
    if (SORT_UNLIKELY(copy == NULL))
    {
        return 0;
    }
    memcpy(copy, arr, num_of_elements * size_of_element);

    bogobogo_sort(copy, num_of_elements - 1, size_of_element, cmp_func_ptr);

    void *last = (char *)copy + (num_of_elements - 1) * size_of_element;
    void *prev_max = (char *)last - size_of_element;

    while (cmp_func_ptr(prev_max, last) > 0)
    {
        shuffle(copy, num_of_elements, size_of_element);
        bogobogo_sort(copy, num_of_elements - 1, size_of_element, cmp_func_ptr);
    }

    int result = (memcmp(arr, copy, num_of_elements * size_of_element) == 0);
    free(copy);
    return result;
}

void bogobogo_sort(void *arr ,size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (num_of_elements <= 1)
    {
        return;
    }
    while (!is_bogobogo_sorted(arr, num_of_elements, size_of_element, cmp_func_ptr))
    {
        shuffle(arr, num_of_elements, size_of_element);
    }
}