/**
 * @file heap_sort.c
 * @brief 힙 정렬 구현부
 */

// maxheap으로 구현

#include <stddef.h>
#include "sorting.h"

#define LEFT_CHILD_IDX(idx) (2 * (idx) + 1)
#define RIGHT_CHILD_IDX(idx) (2 * (idx) + 2)

typedef int (*CmpFunc)(const void *a_ptr, const void *b_ptr);

static void sift_down(void *arr, void *tmp_buf, size_t root_idx, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr);

void heap_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return;
    }

    char stack_buf[SWAP_BUF_SIZE];
    void *tmp_buf = NULL;
    int is_heap = 0;

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
        is_heap = 1;
    }

    /* heapify */
    for (size_t i = (num_of_elements + 1) / 2; i-- > 0;)
    {
        sift_down(arr, tmp_buf, i, num_of_elements, size_of_element, cmp_func_ptr);
    }
    /*---------*/

    void *last_ptr = (char *)arr + ((num_of_elements - 1) * size_of_element);
    for (size_t i = num_of_elements; i > 1; i--)
    {
        generic_swap(arr, last_ptr, tmp_buf, size_of_element);
        last_ptr = (char *)last_ptr - size_of_element;
        sift_down(arr, tmp_buf, 0, i - 1, size_of_element, cmp_func_ptr);
    }

    if (is_heap)
    {
        free(tmp_buf);
    }
}

static void sift_down(void *arr, void *tmp_buf, size_t root_idx, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr)
{
    void *current_ptr = (char *)arr + (root_idx * size_of_element);
    memcpy(tmp_buf, current_ptr, size_of_element);

    size_t child_idx;
    while ((child_idx = LEFT_CHILD_IDX(root_idx)) < num_of_elements) // 왼쪽 자식이 있는 경우 (자식이 하나라도 있는 경우) 반복
    {
        void *child_ptr = (char *)arr + (child_idx * size_of_element);
        if (child_idx + 1 < num_of_elements) // 오른쪽 자식도 있는 경우
        {
            void *right_ptr = (char *)child_ptr + size_of_element;
            if (cmp_func_ptr(child_ptr, right_ptr) < 0)
            {
                child_ptr = right_ptr;
                child_idx++;
            }
        }

        if (cmp_func_ptr(tmp_buf, child_ptr) < 0)
        {
            memcpy(current_ptr, child_ptr, size_of_element);
            root_idx = child_idx;
            current_ptr = child_ptr;
        }
        else
        {
            break;
        }
    }
    memcpy(current_ptr, tmp_buf, size_of_element);
}