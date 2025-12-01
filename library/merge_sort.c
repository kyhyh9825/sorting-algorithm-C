/**
 * @file merge_sort.c
 * @brief 병합 정렬 구현부 (Single, Multi-thread, Double Buffering)
 */

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include "sorting.h"

/* 병렬 처리를 수행할 최소 데이터 개수 (스레드 과생성 방지) */
#define THRESHOLD 16384

typedef int (*CmpFunc)(const void *a_ptr, const void *b_ptr);

/* 멀티스레드 인자 전달용 구조체 */
typedef struct ThreadArgStruct
{
    void *arr;
    void *tmp_arr;
    size_t size_of_element;
    size_t left;
    size_t right;
    CmpFunc cmp_func_ptr;
    int num_threads;
} ThreadArg;

static void internal_merge_sort(void *SORT_RESTRICT arr, void *SORT_RESTRICT tmp_arr, size_t size_of_element, size_t left, size_t right, CmpFunc cmp_func_ptr);
static unsigned __stdcall parallel_internal_sort(void *arg);
static void merge_to_buffer(void *SORT_RESTRICT dest, void *SORT_RESTRICT src, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr);
static inline void merge(void *SORT_RESTRICT arr, void *SORT_RESTRICT tmp_arr, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr);
static int get_thread_count(void);

/* 시스템의 논리 프로세서 개수 반환 */
static int get_thread_count(void)
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

/* [공개 함수] 싱글 스레드 병합 정렬 */
int merge_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return 0;
    }
    void *tmp_arr = malloc(num_of_elements * size_of_element);
    if (SORT_UNLIKELY(tmp_arr == NULL))
    {
        return -1;
    }
    internal_merge_sort(arr, tmp_arr, size_of_element, 0, num_of_elements - 1, cmp_func_ptr);
    free(tmp_arr);
    return 0;
}

/* [공개 함수] 멀티 스레드 병합 정렬 */
int merge_sort_multi(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return 0;
    }
    void *tmp_arr = malloc(num_of_elements * size_of_element);
    if (SORT_UNLIKELY(tmp_arr == NULL))
    {
        return -1;
    }

    /* 시스템에 맞는 적당한 스레드 수 계산 */
    int sys_cpu_count = get_thread_count();
    int cpu_count = (sys_cpu_count >= 8) ? sys_cpu_count - 2 : ((sys_cpu_count >= 4) ? sys_cpu_count - 1 : sys_cpu_count);

    ThreadArg initial_arg = {arr, tmp_arr, size_of_element, 0, num_of_elements - 1, cmp_func_ptr, cpu_count};
    parallel_internal_sort(&initial_arg);
    free(tmp_arr);
    return 0;
}

/* 재귀 분할 정렬 (싱글 스레드) */
static void internal_merge_sort(void *SORT_RESTRICT arr, void *SORT_RESTRICT tmp_arr, size_t size_of_element, size_t left, size_t right, CmpFunc cmp_func_ptr)
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

/* 재귀 분할 정렬 (멀티 스레드) */
static unsigned __stdcall parallel_internal_sort(void *arg)
{
    ThreadArg *arg_ptr = (ThreadArg *)arg;
    if (arg_ptr->left >= arg_ptr->right)
    {
        return 0;
    }
    /* 데이터가 작거나 가용 스레드가 없으면 순차 정렬로 전환 */
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
    parallel_internal_sort(&right_arg); // 오른쪽은 현재 스레드에서 처리
    if (SORT_LIKELY(hThread != 0))
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    else
    {
        /* 스레드 생성 실패 시 현재 스레드에서 순차 처리 */
        internal_merge_sort(left_arg.arr, left_arg.tmp_arr, left_arg.size_of_element, left_arg.left, left_arg.right, left_arg.cmp_func_ptr);
    }
    merge(arg_ptr->arr, arg_ptr->tmp_arr, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->right, arg_ptr->cmp_func_ptr);
    return 0;
}

/* 병합 함수, 요소를 하나씩 병합하지 않고 대소 관계가 연속적인 구간을 찾아 memcpy로 일괄 처리 */
static void merge_to_buffer(void *SORT_RESTRICT dest, void *SORT_RESTRICT src, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr)
{
    char *ptr_left = (char *)src + (left * size_of_element);
    char *ptr_right = (char *)src + ((middle + 1) * size_of_element);

    char *const ptr_left_end = (char *)src + (middle * size_of_element);
    char *const ptr_right_end = (char *)src + (right * size_of_element);

    char *ptr_dest = (char *)dest + (left * size_of_element);

    while (SORT_LIKELY(ptr_left <= ptr_left_end && ptr_right <= ptr_right_end))
    {
        if ((*cmp_func_ptr)(ptr_left, ptr_right) <= 0)
        {
            char *ptr_start = ptr_left;
            /* 연속된 구간 탐색 */
            do
            {
                ptr_left += size_of_element;
            } while (SORT_LIKELY(ptr_left <= ptr_left_end) && (*cmp_func_ptr)(ptr_left, ptr_right) <= 0);

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
            } while (SORT_LIKELY(ptr_right <= ptr_right_end) && (*cmp_func_ptr)(ptr_left, ptr_right) > 0);

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

/* 임시 버퍼에 병합 후 원본으로 복사 */
static inline void merge(void *SORT_RESTRICT arr, void *SORT_RESTRICT tmp_arr, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr)
{
    merge_to_buffer(tmp_arr, arr, size_of_element, left, middle, right, cmp_func_ptr);
    memcpy((char *)arr + (size_of_element * left), (char *)tmp_arr + (size_of_element * left), size_of_element * (right - left + 1));
}

/* --- Ping-Pong (더블 버퍼링) 구현 --- */

/* 멀티스레드 인자 전달용 구조체, 핑퐁 함수에서만 사용됨 */
typedef struct ThreadArgPPStruct
{
    void *dest;
    void *src;
    size_t size_of_element;
    size_t left;
    size_t right;
    CmpFunc cmp_func_ptr;
    int num_threads;
} ThreadArgPP;

static void internal_sort_pp(void *SORT_RESTRICT dest, void *SORT_RESTRICT src, size_t size_of_element, size_t left, size_t right, CmpFunc cmp_func_ptr);
static inline void merge_pp(void *SORT_RESTRICT dest, void *SORT_RESTRICT src, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr);
static unsigned __stdcall parallel_internal_sort_pp(void *arg);

/* [공개 함수] 더블 버퍼링 기반 멀티 스레드 병합 정렬 */
int merge_sort_pp(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{
    if (SORT_UNLIKELY(arr == NULL || num_of_elements <= 1 || size_of_element == 0))
    {
        return 0;
    }
    void *src = malloc(num_of_elements * size_of_element);
    if (SORT_UNLIKELY(src == NULL))
    {
        return -1;
    }
    /* Ping-Pong 로직을 위한 초기 데이터 복사본 생성 */
    memcpy(src, arr, num_of_elements * size_of_element);

    int sys_cpu_count = get_thread_count();
    int cpu_count = (sys_cpu_count >= 8) ? sys_cpu_count - 2 : ((sys_cpu_count >= 4) ? sys_cpu_count - 1 : sys_cpu_count);

    ThreadArgPP initial_arg = {arr, src, size_of_element, 0, num_of_elements - 1, cmp_func_ptr, cpu_count};
    parallel_internal_sort_pp(&initial_arg);
    free(src);
    return 0;
}

/**
 * 재귀 분할 정렬 (Ping-Pong, 멀티스레드)
 * 재귀 깊이에 따라 src와 dest 역할을 교대
 */
static void internal_sort_pp(void *SORT_RESTRICT dest, void *SORT_RESTRICT src, size_t size_of_element, size_t left, size_t right, CmpFunc cmp_func_ptr)
{
    if (left >= right)
    {
        return;
    }
    size_t middle = left + (right - left) / 2;
    /* 다음 단계에서는 src와 dest의 역할을 바꿔 호출 */
    internal_sort_pp(src, dest, size_of_element, left, middle, cmp_func_ptr);
    internal_sort_pp(src, dest, size_of_element, middle + 1, right, cmp_func_ptr);
    merge_pp(dest, src, size_of_element, left, middle, right, cmp_func_ptr);
}

/* Ping-Pong 병합: 단순히 dest 버퍼로 합치기만 하고 src로 복사하지 않음 */
static inline void merge_pp(void *SORT_RESTRICT dest, void *SORT_RESTRICT src, size_t size_of_element, size_t left, size_t middle, size_t right, CmpFunc cmp_func_ptr)
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

    /* 재귀 호출 시 src와 dest 교체 주의 */
    ThreadArgPP left_arg = {arg_ptr->src, arg_ptr->dest, arg_ptr->size_of_element, arg_ptr->left, middle, arg_ptr->cmp_func_ptr, left_threads};
    ThreadArgPP right_arg = {arg_ptr->src, arg_ptr->dest, arg_ptr->size_of_element, middle + 1, arg_ptr->right, arg_ptr->cmp_func_ptr, right_threads};

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, parallel_internal_sort_pp, &left_arg, 0, NULL);
    parallel_internal_sort_pp(&right_arg);
    if (SORT_LIKELY(hThread != 0))
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
