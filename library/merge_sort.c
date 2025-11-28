#if defined(_MSC_VER)
    #define RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
    #define RESTRICT restrict
#else
    #define RESTRICT
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include "sorting.h"

#define THRESHOLD 16384

typedef int (*CmpFunc)(const void *a_ptr, const void *b_ptr);

typedef struct
{
    void *arr;
    void *tmp_arr;
    size_t size_of_element;
    size_t left;
    size_t right;
    CmpFunc cmp_func_ptr;
    int num_threads;
} ThreadArg;

static void internal_merge_sort(void *RESTRICT arr, void *RESTRICT tmp_arr, size_t size_of_element, size_t left, size_t right, CmpFunc cmp_func_ptr);
static unsigned __stdcall parallel_internal_sort(void *arg);
static void merge(void *RESTRICT arr, void *RESTRICT tmp_arr, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr);
static int get_thread_count(void);

static int get_thread_count(void)
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

int merge_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (arr == NULL || num_of_elements <= 1)
    {
        return 0;
    }
    void *tmp_arr = malloc(num_of_elements * size_of_element);
    if (UNLIKELY(tmp_arr == NULL))
    {
        return -1;
    }
    internal_merge_sort(arr, tmp_arr, size_of_element, 0, num_of_elements - 1, cmp_func_ptr);
    free(tmp_arr);
    return 0;
}

int merge_sort_multi(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (arr == NULL || num_of_elements <= 1)
    {
        return 0;
    }
    void *tmp_arr = malloc(num_of_elements * size_of_element);
    if (UNLIKELY(tmp_arr == NULL))
    {
        return -1;
    }
    int sys_cpu_count = get_thread_count();
    int cpu_count = (sys_cpu_count >= 8) ? sys_cpu_count - 2 : ((sys_cpu_count >= 4) ? sys_cpu_count - 1 : sys_cpu_count);
    ThreadArg initial_arg = {arr, tmp_arr, size_of_element, 0, num_of_elements - 1, cmp_func_ptr, cpu_count};
    parallel_internal_sort(&initial_arg);
    free(tmp_arr);
    return 0;
}

static void internal_merge_sort(void *RESTRICT arr, void *RESTRICT tmp_arr, size_t size_of_element, size_t left, size_t right, CmpFunc cmp_func_ptr)
{
    if (left >= right)
    {
        return;
    }
    size_t middle = left + (right - left) / 2;
    internal_merge_sort(arr, tmp_arr, size_of_element, left, middle, cmp_func_ptr);
    internal_merge_sort(arr, tmp_arr, size_of_element, middle + 1, right, cmp_func_ptr);
    merge(arr, tmp_arr, size_of_element, left, middle, right, cmp_func_ptr);
}

static unsigned __stdcall parallel_internal_sort(void *arg)
{
    ThreadArg *arg_ptr = (ThreadArg *)arg;
    if (arg_ptr->left >= arg_ptr->right)
    {
        return 0;
    }
    if (arg_ptr->num_threads <= 1 || arg_ptr->right - arg_ptr->left < THRESHOLD)
    {
        internal_merge_sort(arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, arg_ptr->left, arg_ptr->right, arg_ptr->cmp_func_ptr);
        return 0;
    }

    size_t middle = arg_ptr->left + (arg_ptr->right - arg_ptr->left) / 2;
    int left_threads = arg_ptr->num_threads / 2;
    int right_threads = arg_ptr->num_threads - left_threads;

    ThreadArg left_arg = {arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->cmp_func_ptr, left_threads};
    ThreadArg right_arg = {arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, middle + 1, arg_ptr->right, arg_ptr->cmp_func_ptr, right_threads};

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, parallel_internal_sort, &left_arg, 0, NULL);
    parallel_internal_sort(&right_arg);
    if (hThread != 0)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    else
    {
        internal_merge_sort(left_arg.arr, left_arg.tmp_arr, left_arg.size_of_element, left_arg.left, left_arg.right, left_arg.cmp_func_ptr);
    }
    merge(arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->right, arg_ptr->cmp_func_ptr);
    return 0;
}

static void merge_to_buffer(void *RESTRICT dest, void *RESTRICT src, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr)
{
    char *ptr_left = (char *)src + (left * size_of_element);
    char *ptr_right = (char *)src + ((middle + 1) * size_of_element);

    char *const ptr_left_end = (char *)src + (middle * size_of_element);
    char *const ptr_right_end = (char *)src + (right * size_of_element);

    char *ptr_dest = (char *)dest + (left * size_of_element);

    while (LIKELY(ptr_left <= ptr_left_end && ptr_right <= ptr_right_end))
    {
        if ((*cmp_func_ptr)(ptr_left, ptr_right) <= 0)
        {
            char *ptr_start = ptr_left;
            do
            {
                ptr_left += size_of_element;
            } while (LIKELY(ptr_left <= ptr_left_end) && (*cmp_func_ptr)(ptr_left, ptr_right) <= 0);

            size_t bytes = ptr_left - ptr_start;
            memcpy(ptr_dest, ptr_start, bytes);
            ptr_dest += bytes;
        }
        else
        {
            char *ptr_start = ptr_right;
            do
            {
                ptr_right += size_of_element;
            } while (LIKELY(ptr_right <= ptr_right_end) && (*cmp_func_ptr)(ptr_left, ptr_right) > 0);

            size_t bytes = ptr_right - ptr_start;
            memcpy(ptr_dest, ptr_start, bytes);
            ptr_dest += bytes;
        }
    }

    if (ptr_left > ptr_left_end)
    {
        size_t bytes_remaining = ptr_right_end - ptr_right + size_of_element;
        memcpy(ptr_dest, ptr_right, bytes_remaining);
    }
    else
    {
        size_t bytes_remaining = ptr_left_end - ptr_left + size_of_element;
        memcpy(ptr_dest, ptr_left, bytes_remaining);
    }
}

static void merge(void *RESTRICT arr, void *RESTRICT tmp_arr, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr)
{
    merge_to_buffer(tmp_arr, arr, size_of_element, left, middle, right, cmp_func_ptr);
    memcpy((char *)arr + (size_of_element * left), (char *)tmp_arr + (size_of_element * left), size_of_element * (right - left + 1));
}

typedef struct
{
    void *dest;
    void *src;
    size_t size_of_element;
    size_t left;
    size_t right;
    CmpFunc cmp_func_ptr;
    int num_threads;
} ThreadArgPP;

static void internal_sort_pp(void *RESTRICT dest, void *RESTRICT src, size_t size_of_element, size_t left, size_t right, CmpFunc cmp_func_ptr);
static void merge_pp(void *RESTRICT dest, void *RESTRICT src, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr);
static unsigned __stdcall parallel_internal_sort_pp(void *arg);

int merge_sort_pp(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (arr == NULL || num_of_elements <= 1)
    {
        return 0;
    }
    void *src = malloc(num_of_elements * size_of_element);
    if (UNLIKELY(src == NULL))
    {
        return -1;
    }
    memcpy(src, arr, num_of_elements * size_of_element);
    int sys_cpu_count = get_thread_count();
    int cpu_count = (sys_cpu_count >= 8) ? sys_cpu_count - 2 : ((sys_cpu_count >= 4) ? sys_cpu_count - 1 : sys_cpu_count);
    ThreadArgPP initial_arg = {arr, src, size_of_element, 0, num_of_elements - 1, cmp_func_ptr, cpu_count};
    parallel_internal_sort_pp(&initial_arg);
    free(src);
    return 0;
}

static void internal_sort_pp(void *RESTRICT dest, void *RESTRICT src, size_t size_of_element, size_t left, size_t right, CmpFunc cmp_func_ptr)
{
    if (left >= right)
    {
        return;
    }
    size_t middle = left + (right - left) / 2;
    internal_sort_pp(src, dest, size_of_element, left, middle, cmp_func_ptr);
    internal_sort_pp(src, dest, size_of_element, middle + 1, right, cmp_func_ptr);
    merge_pp(dest, src, size_of_element, left, middle, right, cmp_func_ptr);
}

static void merge_pp(void *RESTRICT dest, void *RESTRICT src, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr)
{
    merge_to_buffer(dest, src, size_of_element, left, middle, right, cmp_func_ptr);
}

static unsigned __stdcall parallel_internal_sort_pp(void *arg)
{
    ThreadArgPP *arg_ptr = (ThreadArgPP *)arg;
    if (arg_ptr->left >= arg_ptr->right)
    {
        return 0;
    }
    if (arg_ptr->num_threads <= 1 || arg_ptr->right - arg_ptr->left < THRESHOLD)
    {
        internal_sort_pp(arg_ptr->dest, arg_ptr->src, arg_ptr->size_of_element, arg_ptr->left, arg_ptr->right, arg_ptr->cmp_func_ptr);
        return 0;
    }

    size_t middle = arg_ptr->left + (arg_ptr->right - arg_ptr->left) / 2;
    int left_threads = arg_ptr->num_threads / 2;
    int right_threads = arg_ptr->num_threads - left_threads;

    ThreadArgPP left_arg = {arg_ptr->src, arg_ptr->dest, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->cmp_func_ptr, left_threads};
    ThreadArgPP right_arg = {arg_ptr->src, arg_ptr->dest, arg_ptr->size_of_element, middle + 1, arg_ptr->right, arg_ptr->cmp_func_ptr, right_threads};

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, parallel_internal_sort_pp, &left_arg, 0, NULL);
    parallel_internal_sort_pp(&right_arg);
    if (hThread != 0)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    else
    {
        internal_sort_pp(left_arg.dest, left_arg.src, left_arg.size_of_element, left_arg.left, left_arg.right, left_arg.cmp_func_ptr);
    }
    merge_pp(arg_ptr->dest, arg_ptr->src, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->right, arg_ptr->cmp_func_ptr);
    return 0;
}
