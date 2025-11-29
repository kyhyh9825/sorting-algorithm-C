/**
 * @file bogo_sort.c
 * @brief 재미를 위한 보고 정렬, 보고보고 정렬 구현부 (Bogo, BogoBogo)
 */

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

/* 내부 함수 프로토타입 */

static inline uint64_t xorshift64(void);
static inline uint64_t mul128(uint64_t a, uint64_t b, uint64_t *low);
static uint64_t random_bounded(uint64_t limit);
static void shuffle(void *SORT_RESTRICT arr, size_t num_of_elements, size_t size_of_element);
static int is_sorted(void *SORT_RESTRICT arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr);
static int is_bogobogo_sorted(void *SORT_RESTRICT arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr);

/* [공개 함수] 보고 정렬 */
void bogo_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
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

/* 64비트 난수 생성 함수 */
static inline uint64_t xorshift64(void)
{
    static uint64_t x = 0;
    if (SORT_UNLIKELY(x == 0))
    {
        x = (uint64_t)time(NULL) ^ 234523452374537324ULL;
    }
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return x;
}

/* 128비트 곱셉 연산 함수 (Lemire 알고리즘용) - 상위 64비트 반환, 하위 64비트는 포인터로 저장 */
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
    /* 32비트 환경에서는 수동으로 64비트 곱셈 처리 */
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

/* Lemire의 방법을 이용한 Bias 없는 난수 생성 */
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

/* 배열 셔플 함수 */
static void shuffle(void *SORT_RESTRICT arr, size_t num_of_elements, size_t size_of_element)
{
    for (size_t i = num_of_elements; i-- > 0;)
    {
        size_t j = (size_t)random_bounded(i + 1);
        char *a_ptr = (char *)arr + (i * size_of_element);
        char *b_ptr = (char *)arr + (j * size_of_element);
        generic_swap(a_ptr, b_ptr, size_of_element);
    }
}

/* 배열 정렬 여부 확인 함수 (O(N)) */
static int is_sorted(void *SORT_RESTRICT arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr)
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

/* 보고보고 정렬 과정에서, 정렬되었는지 확인하기 위해 보고보고 정렬을 이용하여 검증하는 함수 (O(n!^n!)) */
static int is_bogobogo_sorted(void *SORT_RESTRICT arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr)
{
    /* 복사본 생성 */
    void *copy = malloc(num_of_elements * size_of_element);
    if (SORT_UNLIKELY(copy == NULL))
    {
        return 0;
    }
    memcpy(copy, arr, num_of_elements * size_of_element);

    /* 복사본의 앞 n - 1개를 보고보고 정렬 */
    bogobogo_sort(copy, num_of_elements - 1, size_of_element, cmp_func_ptr);

    void *last = (char *)copy + (num_of_elements - 1) * size_of_element;
    void *prev_max = (char *)last - size_of_element;

    /* n번째 요소가 정렬될 때까지 셔플 및 재정렬 */
    while (cmp_func_ptr(prev_max, last) > 0)
    {
        shuffle(copy, num_of_elements, size_of_element);
        bogobogo_sort(copy, num_of_elements - 1, size_of_element, cmp_func_ptr);
    }

    /* 원본과 정렬된 복사본 비교 */
    int result = (memcmp(arr, copy, num_of_elements * size_of_element) == 0);
    free(copy);
    return result;
}

/* [공개 함수] 보고보고 정렬 */
void bogobogo_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
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