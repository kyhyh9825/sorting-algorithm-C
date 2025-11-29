/**
 * @file insertion_sort.c
 * @brief 삽입 정렬 구현부 (Normal, Binary)
 */

#include <stdio.h>

void insertion_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return;
    }
    void *cmp = arr;
    void *current = (char *)arr + size_of_element;
    for (size_t i = 1; i < num_of_elements; i++)
    {
        
    }
}

void binary_insertion_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{

}