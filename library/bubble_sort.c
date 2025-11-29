#include <stdio.h>
#include "sorting.h"

void bubble_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (num_of_elements <= 1)
    {
        return;
    }
    for (size_t i = num_of_elements - 1; i > 0; i--)
    {
        int is_swapped = 0;
        for (size_t j = 0; j < i; j++)
        {
            void *current_ptr = (char *)arr + (j * size_of_element);
            void *next_ptr = (char *)current_ptr + size_of_element;

            if (cmp_func_ptr(current_ptr, next_ptr) > 0)
            {
                generic_swap(current_ptr, next_ptr, size_of_element);
                is_swapped = 1;
            }
        }
        if (SORT_UNLIKELY(!is_swapped))
        {
            return;
        }
    }
}