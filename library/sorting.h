/**
 * @file sorting.h 
 * 
 * @brief 다양한 정렬 알고리즘을 포함하는 라이브러리
 * 
 * void 포인터를 이용한 제네릭으로 구현되어 구조체를 포함하는 다양한 자료형 지원
 * 
 * @note Windows가 아닌 환경에서는 실행할 수 없음
 * 
 * */

#ifndef SORTING_H
#define SORTING_H

#if defined(_MSC_VER)
    #define SORT_RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
    #define SORT_RESTRICT restrict
#else
    #define SORT_RESTRICT
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define SORT_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define SORT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define SORT_LIKELY(x)   (x)
    #define SORT_UNLIKELY(x) (x)
#endif

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#define SWAP_BUF_SIZE 256

typedef struct GulagStruct
{
    void *location;
    size_t count;
} Gulag;

static inline void generic_swap(void *a_ptr, void *b_ptr, size_t size_of_element)
{
    if (a_ptr == b_ptr)
    {
        return;
    }
    if (SORT_LIKELY(size_of_element <= SWAP_BUF_SIZE))
    {
        char tmp[SWAP_BUF_SIZE];
        memcpy(tmp, a_ptr, size_of_element);
        memcpy(a_ptr, b_ptr, size_of_element);
        memcpy(b_ptr, tmp, size_of_element);
    }
    else
    {
        void *tmp = malloc(size_of_element);
        if (SORT_UNLIKELY(tmp == NULL))
        {
            return;
        }
        memcpy(tmp, a_ptr, size_of_element);
        memcpy(a_ptr, b_ptr, size_of_element);
        memcpy(b_ptr, tmp, size_of_element);
        free(tmp);
    }
}

/**
 * @brief 싱글 스레드 병합 정렬
 * 
 * @param arr 정렬할 배열의 시작 주소
 * @param num_of_elements 요소의 총 개수
 * @param size_of_element 요소 하나의 크기 (sizeof(Type))
 * @param cmp_func_ptr 비교 함수 포인터 (qsort에 사용되는 비교 함수와 동일하게 작동)
 * 
 * @return 정렬에 필요한 메모리 할당에 실패하면 -1을, 성공하면 0을 반환
 * 
 */
int merge_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

/**
 * @brief 멀티 스레드 병합 정렬
 * 
 * @param arr 정렬할 배열의 시작 주소
 * @param num_of_elements 요소의 총 개수
 * @param size_of_element 요소 하나의 크기 (sizeof(Type))
 * @param cmp_func_ptr 비교 함수 포인터 (qsort에 사용되는 비교 함수와 동일하게 작동)
 * 
 * @return 정렬에 필요한 메모리 할당에 실패하면 -1을, 성공하면 0을 반환
 * 
 */
int merge_sort_multi(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

/**
 * @brief 더블 버퍼링 이용 멀티 스레드 병합 정렬
 * 
 * @param arr 정렬할 배열의 시작 주소
 * @param num_of_elements 요소의 총 개수
 * @param size_of_element 요소 하나의 크기 (sizeof(Type))
 * @param cmp_func_ptr 비교 함수 포인터 (qsort에 사용되는 비교 함수와 동일하게 작동)
 * 
 * @return 정렬에 필요한 메모리 할당에 실패하면 -1을, 성공하면 0을 반환
 * 
 */
int merge_sort_pp(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

/* heap_sort 만드는중 */
// void heap_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

/* bogo_sort 만드는중 */
void bogo_sort(void *arr ,size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

void bogobogo_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

/**
 * @brief 스탈린 정렬: 첫 원소와 다음 원소를 비교하여 순서에 맞지 않으면 다음 원소를 굴라그(gulag) 배열로 보내 숙청
 * 
 * 기존 배열에는 정렬된 원소만 남음
 * 
 * @return 굴라그 배열의 주소 포인터 (void *)location과 숙청당한 원소의 수 size_t count를 갖는 구조체 포인터를 반환
 * 
 */
Gulag *stalin_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*purge_func_ptr)(const void *a_ptr, const void *b_ptr));

#endif // SORTING_H