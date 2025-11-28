#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define THRESHOLD 16384

typedef struct
{
    void *dest;
    void *src;
    size_t size_of_element;
    size_t left;
    size_t right;
    int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr);
    int num_threads;
} ThreadArg;

int get_thread_count(void);
void merge_sort(void *dest, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
void m_sort(void *dest, void *src, size_t size_of_element, size_t left, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
void merge(void *dest, void *src, size_t size_of_element, size_t left, size_t middle, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
unsigned __stdcall parallel_m_sort(void *arg);

int get_thread_count(void)
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

void merge_sort(void *dest, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (num_of_elements <= 1) return;
    void *src = malloc(num_of_elements * size_of_element);
    if (src == NULL)
    {
        printf("Error: Memory allocation failed in merge_sort\n");
        return;
    }
    memcpy(src, dest, num_of_elements * size_of_element);
    int max_threads = (get_thread_count() > 2) ? get_thread_count() - 2 : 1;
    ThreadArg initial_arg = {dest, src, size_of_element, 0, num_of_elements - 1, cmp_func_ptr, max_threads};
    parallel_m_sort(&initial_arg);
    free(src);
}

void m_sort(void *dest, void *src, size_t size_of_element, size_t left, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (left >= right) return;

    size_t middle = left + (right - left) / 2;
    m_sort(src, dest, size_of_element, left, middle, cmp_func_ptr);
    m_sort(src, dest, size_of_element, middle + 1, right, cmp_func_ptr);
    merge(dest, src, size_of_element, left, middle, right, cmp_func_ptr);
}

void merge(void *dest, void *src, size_t size_of_element, size_t left, size_t middle, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    size_t left_index = left;
    size_t right_index = middle + 1;
    size_t dest_index = left;

    while (left_index <= middle && right_index <= right)
    {
        if ((*cmp_func_ptr)((char *)src + (size_of_element * left_index), (char *)src + (size_of_element * right_index)) <= 0)
        {
            memcpy((char *)dest + (size_of_element * dest_index++), (char *)src + (size_of_element * left_index++), size_of_element);
        }
        else
        {
            memcpy((char *)dest + (size_of_element * dest_index++), (char *)src + (size_of_element * right_index++), size_of_element);
        }
    }
    if (left_index > middle)
    {
        memcpy((char *)dest + (size_of_element * dest_index), (char *)src + (size_of_element * right_index), size_of_element * (right - right_index + 1));
    }
    else
    {
        memcpy((char *)dest + (size_of_element * dest_index), (char *)src + (size_of_element * left_index), size_of_element * (middle - left_index + 1));
    }
}

unsigned __stdcall parallel_m_sort(void *arg)
{
    ThreadArg *arg_ptr = (ThreadArg *)arg;
    if (arg_ptr->left >= arg_ptr->right) return 0;

    if (arg_ptr->num_threads <= 1 || arg_ptr->right - arg_ptr->left < THRESHOLD)
    {
        m_sort(arg_ptr->dest, arg_ptr->src, arg_ptr->size_of_element, arg_ptr->left, arg_ptr->right, arg_ptr->cmp_func_ptr);
        return 0;
    }

    size_t middle = arg_ptr->left + (arg_ptr->right - arg_ptr->left) / 2;
    int left_threads = arg_ptr->num_threads / 2;
    int right_threads = arg_ptr->num_threads - left_threads;

    ThreadArg left_arg = {arg_ptr->src, arg_ptr->dest, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->cmp_func_ptr, left_threads};
    ThreadArg right_arg = {arg_ptr->src, arg_ptr->dest, arg_ptr->size_of_element, middle + 1, arg_ptr->right, arg_ptr->cmp_func_ptr, right_threads};

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, parallel_m_sort, &left_arg, 0, NULL);
    parallel_m_sort(&right_arg);
    if (hThread != 0)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    else
    {
        m_sort(left_arg.dest, left_arg.src, left_arg.size_of_element, left_arg.left, left_arg.right, left_arg.cmp_func_ptr);
    }
    merge(arg_ptr->dest, arg_ptr->src, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->right, arg_ptr->cmp_func_ptr);
    return 0;
}