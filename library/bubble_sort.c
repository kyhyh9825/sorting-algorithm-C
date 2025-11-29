/**
 * @file bubble_sort.c
 * @brief 버블 정렬 구현부
 */

#include <stdio.h>
#include "sorting.h"

void bubble_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return;
    }
    for (size_t i = num_of_elements - 1; i > 0; i--)
    {
        int is_swapped = 0;
        void *current = arr;
        void *next = (char *)arr + size_of_element;
        for (size_t j = 0; j < i; j++)
        {
            if (cmp_func_ptr(current, next) > 0)
            {
                generic_swap(current, next, size_of_element);
                is_swapped = 1;
            }
            current = (char *)current + size_of_element;
            next = (char *)next + size_of_element;
        }
        if (SORT_UNLIKELY(!is_swapped))
        {
            return;
        }
    }
}