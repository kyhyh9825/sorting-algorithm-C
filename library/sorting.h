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

#include <stddef.h>

/**
 * @brief 싱글 스레드 병합 정렬
 * 
 * @param arr 정렬할 배열의 시작 주소
 * @param num_of_elements 요소의 총 개수
 * @param size_of_elements 요소 하나의 크기 (sizeof(Type))
 * @param cmp_func_ptr 비교 함수 포인터 (qsort에 사용되는 비교 함수와 동일하게 작동)
 */
void merge_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

/**
 * @brief 멀티 스레드 병합 정렬
 * 
 * @param arr 정렬할 배열의 시작 주소
 * @param num_of_elements 요소의 총 개수
 * @param size_of_elements 요소 하나의 크기 (sizeof(Type))
 * @param cmp_func_ptr 비교 함수 포인터 (qsort에 사용되는 비교 함수와 동일하게 작동)
 */
void merge_sort_multi(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

/**
 * @brief 더블 버퍼링 이용 멀티 스레드 병합 정렬
 * 
 * @param arr 정렬할 배열의 시작 주소
 * @param num_of_elements 요소의 총 개수
 * @param size_of_elements 요소 하나의 크기 (sizeof(Type))
 * @param cmp_func_ptr 비교 함수 포인터 (qsort에 사용되는 비교 함수와 동일하게 작동)
 */
void merge_sort_pp(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

/* heap_sort 만드는중 */
// void heap_sort(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

#endif // SORTING_H