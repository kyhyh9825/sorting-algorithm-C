#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sorting.h"

static void compress_arr(void *SORT_RESTRICT arr, int *is_purged, size_t num_of_elements, size_t size_of_element);

Gulag *stalin_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*purge_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    /* 수용소 할당 후 청소(calloc) */
    Gulag *gulag = (Gulag *)calloc(1, sizeof(Gulag));
    gulag->location = malloc(num_of_elements * size_of_element);
    // gulag->count = 0;
    if (SORT_UNLIKELY(gulag->location == NULL))
    {
        free(gulag);
        return NULL;
    }

    void *current_gulag_ptr = gulag->location;
    int *is_purged = (int *)calloc(num_of_elements, sizeof(int));
    if (SORT_UNLIKELY(is_purged == NULL))
    {
        free(gulag->location);
        free(gulag);
        return NULL;
    }

    void *max_element = arr;
    for (size_t i = 1; i < num_of_elements; i++)
    {
        void *current = (char *)arr + (i * size_of_element);
        if (purge_func_ptr(max_element, current) > 0)
        {
            memcpy(current_gulag_ptr, current, size_of_element); // 숙청
            current_gulag_ptr = (char *)current_gulag_ptr + size_of_element;
            is_purged[i] = 1;
            gulag->count++;
        }
        else
        {
            max_element = current;
        }
    }
    compress_arr(arr, is_purged, num_of_elements, size_of_element);
    free(is_purged);

    if (gulag->count > 0)
    {
        void *shrunk = realloc(gulag->location, gulag->count * size_of_element);
        if (SORT_LIKELY(shrunk != NULL))
        {
            gulag->location = shrunk;
        }
    }
    else
    {
        free(gulag->location);
        gulag->location = NULL;
    }
    return gulag;
}

static void compress_arr(void *SORT_RESTRICT arr, int *is_purged, size_t num_of_elements, size_t size_of_element)
{
    void *write_ptr = arr;
    void *read_ptr = arr;

    for (size_t i  = 0; i < num_of_elements; i++)
    {
        if (!is_purged[i])
        {
            if (write_ptr != read_ptr)
            {
                memcpy(write_ptr, read_ptr, size_of_element);
            }
            write_ptr = (char *)write_ptr + size_of_element;
        }
        read_ptr = (char *)read_ptr + size_of_element;
    }
}