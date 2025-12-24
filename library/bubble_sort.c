/**
 * @file bubble_sort.c
 * @brief 버블 정렬 구현부
 */

#include <stdlib.h>
#include "sorting.h"

/* [공개 함수] 버블 정렬 */
void bubble_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return;
    }

    char stack_buf[SWAP_BUF_SIZE];
    void *tmp_buf = NULL;
    int is_malloc_used = 0;

    if (SORT_LIKELY(size_of_element <= SWAP_BUF_SIZE))
    {
        tmp_buf = stack_buf;
    }
    else
    {
        tmp_buf = malloc(size_of_element);
        if (SORT_UNLIKELY(tmp_buf == NULL))
        {
            return;
        }
        is_malloc_used = 1;
    }

    for (size_t i = num_of_elements - 1; i > 0; i--)
    {
        int is_swapped = 0;
        char *current = arr;
        char *next = (char *)arr + size_of_element;
        for (size_t j = 0; j < i; j++)
        {
            if (cmp_func(current, next) > 0)
            {
                generic_swap(current, next, tmp_buf, size_of_element);
                is_swapped = 1;
            }
            current += size_of_element;
            next += size_of_element;
        }
        if (SORT_UNLIKELY(!is_swapped))
        {
            if (is_malloc_used)
            {
                free(tmp_buf);
            }
            return;
        }
    }
    if (is_malloc_used)
    {
        free(tmp_buf);
    }
}