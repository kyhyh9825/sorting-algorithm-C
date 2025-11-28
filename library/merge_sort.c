#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include "../library/sorting.h"

#define THRESHOLD 16384

typedef struct
{
    void *arr;
    void *tmp_arr;
    size_t size_of_element;
    size_t left;
    size_t right;
    int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr);
    int num_threads;
} ThreadArg;

static void m_sort(void *arr, void *tmp_arr, size_t size_of_element, size_t left, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
static unsigned __stdcall parallel_m_sort(void *arg);
static void merge(void *arr, void *tmp_arr, size_t size_of_element, size_t left, size_t middle, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
static int get_thread_count(void);

static int get_thread_count(void)
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

void merge_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (num_of_elements <= 1) return;
    void *tmp_arr = malloc(num_of_elements * size_of_element);
    if (tmp_arr == NULL)
    {
        printf("Error: Memory allocation failed in merge_sort_single\n");
        return;
    }
    m_sort(arr, tmp_arr, size_of_element, 0, num_of_elements - 1, cmp_func_ptr);
    free(tmp_arr);
}

void merge_sort_multi(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (num_of_elements <= 1) return;
    void *tmp_arr = malloc(num_of_elements * size_of_element);
    if (tmp_arr == NULL)
    {
        printf("Error: Memory allocation failed in merge_sort_multi\n");
        return;
    }
    int cpu_count = (get_thread_count() > 2) ? get_thread_count() - 2 : 1;
    ThreadArg initial_arg = {arr, tmp_arr, size_of_element, 0, num_of_elements - 1, cmp_func_ptr, cpu_count};
    parallel_m_sort(&initial_arg);
    free(tmp_arr);
}

static void m_sort(void *arr, void *tmp_arr, size_t size_of_element, size_t left, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (left >= right) return;

    size_t middle = left + (right - left) / 2;
    m_sort(arr, tmp_arr, size_of_element, left, middle, cmp_func_ptr);
    m_sort(arr, tmp_arr, size_of_element, middle + 1, right, cmp_func_ptr);
    merge(arr, tmp_arr, size_of_element, left, middle, right, cmp_func_ptr);
}

static unsigned __stdcall parallel_m_sort(void *arg)
{
    ThreadArg *arg_ptr = (ThreadArg *)arg;
    if (arg_ptr->left >= arg_ptr->right) return 0;

    if (arg_ptr->num_threads <= 1 || arg_ptr->right - arg_ptr->left < THRESHOLD)
    {
        m_sort(arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, arg_ptr->left, arg_ptr->right, arg_ptr->cmp_func_ptr);
        return 0;
    }

    size_t middle = arg_ptr->left + (arg_ptr->right - arg_ptr->left) / 2;
    int left_threads = arg_ptr->num_threads / 2;
    int right_threads = arg_ptr->num_threads - left_threads;

    ThreadArg left_arg = {arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->cmp_func_ptr, left_threads};
    ThreadArg right_arg = {arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, middle + 1, arg_ptr->right, arg_ptr->cmp_func_ptr, right_threads};

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, parallel_m_sort, &left_arg, 0, NULL);
    parallel_m_sort(&right_arg);
    if (hThread != 0)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    else
    {
        m_sort(left_arg.arr, left_arg.tmp_arr, left_arg.size_of_element, left_arg.left, left_arg.right, left_arg.cmp_func_ptr);
    }
    merge(arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->right, arg_ptr->cmp_func_ptr);
    return 0;
}

static void merge(void *arr, void *tmp_arr, size_t size_of_element, size_t left, size_t middle, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
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

typedef struct
{
    void *dest;
    void *src;
    size_t size_of_element;
    size_t left;
    size_t right;
    int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr);
    int num_threads;
} ThreadArgPP;

static void m_sort_pp(void *dest, void *src, size_t size_of_element, size_t left, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
static void merge_pp(void *dest, void *src, size_t size_of_element, size_t left, size_t middle, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));
static unsigned __stdcall parallel_m_sort_pp(void *arg);

void merge_sort_pp(void *dest, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (num_of_elements <= 1) return;
    void *src = malloc(num_of_elements * size_of_element);
    if (src == NULL)
    {
        printf("Error: Memory allocation failed in merge_sort_pp\n");
        return;
    }
    memcpy(src, dest, num_of_elements * size_of_element);
    int cpu_count = (get_thread_count() > 2) ? get_thread_count() - 2 : 1;
    ThreadArgPP initial_arg = {dest, src, size_of_element, 0, num_of_elements - 1, cmp_func_ptr, cpu_count};
    parallel_m_sort_pp(&initial_arg);
    free(src);
}

static void m_sort_pp(void *dest, void *src, size_t size_of_element, size_t left, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (left >= right) return;

    size_t middle = left + (right - left) / 2;
    m_sort_pp(src, dest, size_of_element, left, middle, cmp_func_ptr);
    m_sort_pp(src, dest, size_of_element, middle + 1, right, cmp_func_ptr);
    merge_pp(dest, src, size_of_element, left, middle, right, cmp_func_ptr);
}

static void merge_pp(void *dest, void *src, size_t size_of_element, size_t left, size_t middle, size_t right, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
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

static unsigned __stdcall parallel_m_sort_pp(void *arg)
{
    ThreadArgPP *arg_ptr = (ThreadArgPP *)arg;
    if (arg_ptr->left >= arg_ptr->right) return 0;

    if (arg_ptr->num_threads <= 1 || arg_ptr->right - arg_ptr->left < THRESHOLD)
    {
        m_sort_pp(arg_ptr->dest, arg_ptr->src, arg_ptr->size_of_element, arg_ptr->left, arg_ptr->right, arg_ptr->cmp_func_ptr);
        return 0;
    }

    size_t middle = arg_ptr->left + (arg_ptr->right - arg_ptr->left) / 2;
    int left_threads = arg_ptr->num_threads / 2;
    int right_threads = arg_ptr->num_threads - left_threads;

    ThreadArgPP left_arg = {arg_ptr->src, arg_ptr->dest, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->cmp_func_ptr, left_threads};
    ThreadArgPP right_arg = {arg_ptr->src, arg_ptr->dest, arg_ptr->size_of_element, middle + 1, arg_ptr->right, arg_ptr->cmp_func_ptr, right_threads};

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, parallel_m_sort_pp, &left_arg, 0, NULL);
    parallel_m_sort_pp(&right_arg);
    if (hThread != 0)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    else
    {
        m_sort_pp(left_arg.dest, left_arg.src, left_arg.size_of_element, left_arg.left, left_arg.right, left_arg.cmp_func_ptr);
    }
    merge_pp(arg_ptr->dest, arg_ptr->src, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->right, arg_ptr->cmp_func_ptr);
    return 0;
}
