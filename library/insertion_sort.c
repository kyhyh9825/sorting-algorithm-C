/**
 * @file insertion_sort.c
 * @brief 삽입 정렬 구현부 (Normal, Binary)
 */

#include <stdio.h>
#include <string.h>
#include "sorting.h"

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
        if (tmp == NULL)
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
        if (pos != current)
        {
            memcpy(tmp, current, size_of_element);
            memmove((char *)pos + size_of_element, pos, (char *)current - pos);
            memcpy(pos, tmp, size_of_element);
        }
        current = (char *)current + size_of_element;
    }

    if (is_heap) // tmp를 동적할당 하였다면 메모리 해제
    {
        free(tmp);
    }
}

void binary_insertion_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{

}