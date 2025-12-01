/**
 * @file selection_sort.c
 * @brief 선택 정렬 구현부
 */

 #include <stdio.h>
 #include "../library/sorting.h"

 void selection_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
 {
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return;
    }

    void *select = arr;
    void *current = arr;
    for (size_t i = 0; i < num_of_elements; i++)
    {
        void *scan = (char *)select + size_of_element;
        for (size_t j = i + 1; j < num_of_elements; j++)
        {
            if (cmp_func_ptr(select, scan) > 0)
            {
                select = scan;
            }
            generic_swap(current, select, size_of_element); 
            scan = (char *)scan + size_of_element;
        }
        select = (char *)select + size_of_element;
        current = (char *)current + size_of_element;
    }
}