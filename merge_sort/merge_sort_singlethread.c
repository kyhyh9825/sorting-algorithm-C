#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void merge_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
void m_sort(void *arr, void *tmp_arr, size_t size_of_element, size_t left, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
void merge(void *arr, void *tmp_arr, size_t size_of_element, size_t left, size_t middle, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

void merge_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (num_of_elements <= 1) return;
    void *tmp_arr = malloc(num_of_elements * size_of_element);
    if (tmp_arr == NULL)
    {
        printf("Error: Memory allocation failed in merge_sort\n");
        return;
    }
    m_sort(arr, tmp_arr, size_of_element, 0, num_of_elements - 1, cmp_func_ptr);
    free(tmp_arr);
}

void m_sort(void *arr, void *tmp_arr, size_t size_of_element, size_t left, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (left >= right) return;

    size_t middle = left + (right - left) / 2;
    m_sort(arr, tmp_arr, size_of_element, left, middle, cmp_func_ptr);
    m_sort(arr, tmp_arr, size_of_element, middle + 1, right, cmp_func_ptr);
    merge(arr, tmp_arr, size_of_element, left, middle, right, cmp_func_ptr);
}

void merge(void *arr, void *tmp_arr, size_t size_of_element, size_t left, size_t middle, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    size_t left_index = left;
    size_t right_index = middle + 1;
    size_t tmp_arr_index = left;

    while (left_index <= middle && right_index <= right)
    {
        if ((*cmp_func_ptr)((char *)arr + (size_of_element * left_index), (char *)arr + (size_of_element * right_index)) <= 0)
        {
            memcpy((char *)tmp_arr + (size_of_element * tmp_arr_index++), (char *)arr + (size_of_element * left_index++), size_of_element);
        }
        else
        {
            memcpy((char *)tmp_arr + (size_of_element * tmp_arr_index++), (char *)arr + (size_of_element * right_index++), size_of_element);
        }
    }
    if (left_index > middle)
    {
        memcpy((char *)tmp_arr + (size_of_element * tmp_arr_index), (char *)arr + (size_of_element * right_index), size_of_element * (right - right_index + 1));
    }
    else
    {
        memcpy((char *)tmp_arr + (size_of_element * tmp_arr_index), (char *)arr + (size_of_element * left_index), size_of_element * (middle - left_index + 1));
    }
    memcpy((char *)arr + (size_of_element * left), (char *)tmp_arr + (size_of_element * left), size_of_element * (right - left + 1));
}

int main(void)
{
    system("pause");
    return 0;
}