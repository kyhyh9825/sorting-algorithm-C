/**
 * @file insertion_sort.c
 * @brief 삽입 정렬 구현부 (Normal, Binary)
 */

#include <stdio.h>
#include <string.h>
#include "sorting.h"

typedef int (*CmpFunc)(const void *a_ptr, const void *b_ptr);

static void *binary_pos_search(void *arr, void *value, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr);

/* [공개 함수] 삽입 정렬 */
void insertion_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return;
    }

    char stack_buf[SWAP_BUF_SIZE];
    void *tmp = NULL;
    int is_heap = 0;

    if (SORT_LIKELY(size_of_element <= SWAP_BUF_SIZE))
    {
        tmp = stack_buf;
    }
    else
    {
        tmp = malloc(size_of_element);
        if (SORT_UNLIKELY(tmp == NULL))
        {
            return;
        }
        is_heap = 1;
    }

    void *current = (char *)arr + size_of_element;
    for (size_t i = 1; i < num_of_elements; i++) // n - 1번 반복
    {
        void *scan = (char *)current - size_of_element;
        void *pos = arr;
        for (size_t j = i; j-- > 0;) // i번 반복
        {
            if (cmp_func_ptr(scan, current) <= 0) // 삽입 조건
            {
                pos = (char *)scan + size_of_element;
                break;
            }
            scan = (char *)scan - size_of_element;
        }
        if (SORT_LIKELY(pos != current))
        {
            memcpy(tmp, current, size_of_element);
            memmove((char *)pos + size_of_element, pos, (char *)current - (char *)pos);
            memcpy(pos, tmp, size_of_element);
        }
        current = (char *)current + size_of_element;
    }

    if (is_heap) // tmp를 힙 영역에 동적할당 하였다면 메모리 해제
    {
        free(tmp);
    }
}

/* [공개 함수] 이진 삽입 정렬 */
void insertion_sort_binary(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return;
    }

    char stack_buf[SWAP_BUF_SIZE];
    void *tmp = NULL;
    int is_heap = 0;

    if (SORT_LIKELY(size_of_element <= SWAP_BUF_SIZE))
    {
        tmp = stack_buf;
    }
    else
    {
        tmp = malloc(size_of_element);
        if (SORT_UNLIKELY(tmp == NULL))
        {
            return;
        }
        is_heap = 1;
    }

    void *current = (char *)arr + size_of_element;
    for (size_t i = 1; i < num_of_elements; i++)
    {
        void *pos = binary_pos_search(arr, current, i, size_of_element, cmp_func_ptr);
        if (SORT_LIKELY(pos != current))
        {
            memcpy(tmp, current, size_of_element);
            memmove((char *)pos + size_of_element, pos, (char *)current - (char *)pos);
            memcpy(pos, tmp, size_of_element);
        }
        current = (char *)current + size_of_element;
    }

    if (is_heap) // tmp를 힙 영역에 동적할당 하였다면 메모리 해제
    {
        free(tmp);
    }
}

/* 이진 삽입 정렬에서 삽입할 자리를 찾을 때 이진 탐색을 이용하여 찾는 함수 */
static void *binary_pos_search(void *arr, void *value, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr)
{
    size_t lo = 0;
    size_t hi = (num_of_elements - 1) + 1; // 자기 자신이 가장 큰 경우도 확인해야 하므로 마지막 인덱스에서 1 더함

    while (lo < hi)
    {
        size_t mid = lo + (hi - lo) / 2;
        void *mid_ptr = (char *)arr + (mid * size_of_element);
        if (cmp_func_ptr(value, mid_ptr) < 0) // mid 번째 값보다 작으면
        {
            hi = mid;
        }
        else // mid 번째 값보다 같거나 크면
        {
            lo = mid + 1;
        }
    }
    return (char *)arr + lo * size_of_element;
}