#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../library/sorting.h"

// ==========================================
// 1. 설정 및 데이터 정의
// ==========================================
#define INT_COUNT    200000 // 정수 테스트 개수
#define STRUCT_COUNT 120000 // 구조체 테스트 개수 (이동 비용 감안하여 조절)

typedef struct
{
    int id;             // 4 bytes
    double score;       // 8 bytes
    char name[12];      // 12 bytes (Padding 포함 24 bytes 예상)
} Student;

// ==========================================
// 2. 비교 함수
// ==========================================

// [Integer] 오름차순
int cmp_int(const void *a, const void *b)
{
    const int arg1 = *(const int *)a;
    const int arg2 = *(const int *)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

// [Student] 점수 내림차순 (높은 점수 1등), 점수 같으면 ID 오름차순
int cmp_student(const void *a, const void *b)
{
    const Student *s1 = (const Student *)a;
    const Student *s2 = (const Student *)b;

    if (s1->score > s2->score) return -1; // 내림차순
    if (s1->score < s2->score) return 1;
    
    if (s1->id < s2->id) return -1;       // 오름차순
    if (s1->id > s2->id) return 1;

    return 0;
}

// ==========================================
// 3. 유틸리티 함수
// ==========================================

void verify_sorted(void *arr, size_t n, size_t size, int (*cmp)(const void *, const void *), const char *algo)
{
    char *ptr = (char *)arr;
    for (size_t i = 0; i < n - 1; i++)
    {
        if (cmp(ptr + (i * size), ptr + ((i + 1) * size)) > 0)
        {
            printf(" [FAIL] %-16s : Not sorted at index %zu\n", algo, i);
            return;
        }
    }
    printf(" [PASS] %-16s : Verified.\n", algo);
}

// 단일 케이스 실행 함수
void run_case(const char *algo_name, void *original, size_t n, size_t size, int (*cmp)(const void *, const void *), 
              void (*sort_func)(void *, size_t, size_t, int (*)(const void *, const void *)))
{
    void *arr = malloc(n * size);
    if (!arr) return;

    memcpy(arr, original, n * size); // 데이터 복사

    clock_t start = clock();
    sort_func(arr, n, size, cmp); // 정렬 수행
    clock_t end = clock();

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  %-16s : %7.4f sec", algo_name, time_taken);
    
    // 검증 결과 출력 (줄바꿈 포함)
    char temp_name[30];
    sprintf(temp_name, "(%s)", algo_name); // verify 로그용 이름
    // verify_sorted 안에서 줄바꿈 함하지만 포맷 맞추기 위해 꼼수
    printf("\n");
    verify_sorted(arr, n, size, cmp, algo_name);

    free(arr);
}

// 퀵소트 래퍼 (표준 라이브러리라 시그니처가 다름)
void wrapper_qsort(void *arr, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    qsort(arr, n, size, cmp);
}

// 3종 세트(Normal, Binary, Qsort) 한 번에 돌리기
void run_comparison_set(const char *data_type, const char *pattern, void *data, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    printf("\n==========================================================\n");
    printf(" [%s] Pattern: %s (N=%zu)\n", data_type, pattern, n);
    printf("==========================================================\n");

    run_case("Normal Insertion", data, n, size, cmp, insertion_sort);
    run_case("Binary Insertion", data, n, size, cmp, insertion_sort_binary);
    run_case("std::qsort",       data, n, size, cmp, wrapper_qsort);
}

// ==========================================
// 4. 메인 (시나리오 생성)
// ==========================================
int main(void)
{
    srand((unsigned int)time(NULL));

    // ---------------------------------------------------------
    // PART 1: Integer Tests (50,000개)
    // ---------------------------------------------------------
    int *int_raw = (int *)malloc(INT_COUNT * sizeof(int));
    
    // 1-1. Random
    for (size_t i = 0; i < INT_COUNT; i++) int_raw[i] = rand();
    run_comparison_set("Integer", "Random", int_raw, INT_COUNT, sizeof(int), cmp_int);

    // 1-2. Sorted (0, 1, 2 ...)
    for (size_t i = 0; i < INT_COUNT; i++) int_raw[i] = (int)i;
    run_comparison_set("Integer", "Sorted", int_raw, INT_COUNT, sizeof(int), cmp_int);

    // 1-3. Reverse Sorted (N, N-1 ...)
    for (size_t i = 0; i < INT_COUNT; i++) int_raw[i] = (int)(INT_COUNT - i);
    run_comparison_set("Integer", "Reverse", int_raw, INT_COUNT, sizeof(int), cmp_int);

    free(int_raw);

    // ---------------------------------------------------------
    // PART 2: Structure Tests (30,000개) - 무거움
    // ---------------------------------------------------------
    Student *stu_raw = (Student *)malloc(STRUCT_COUNT * sizeof(Student));

    // 2-1. Random
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        stu_raw[i].id = (int)i;
        stu_raw[i].score = ((double)rand() / RAND_MAX) * 100.0;
    }
    run_comparison_set("Struct", "Random", stu_raw, STRUCT_COUNT, sizeof(Student), cmp_student);

    // 2-2. Sorted (Score: 100 -> 0) *cmp_student는 내림차순이 정렬된 상태임*
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        stu_raw[i].id = (int)i;
        // i가 커질수록 점수가 작아지게 배치 -> 내림차순 정렬 상태
        stu_raw[i].score = 100.0 - ((double)i / STRUCT_COUNT * 100.0);
    }
    run_comparison_set("Struct", "Sorted", stu_raw, STRUCT_COUNT, sizeof(Student), cmp_student);

    // 2-3. Reverse Sorted (Score: 0 -> 100) *내림차순 정렬의 반대*
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        stu_raw[i].id = (int)i;
        // i가 커질수록 점수가 커지게 배치 -> 오름차순(역정렬) 상태
        stu_raw[i].score = ((double)i / STRUCT_COUNT * 100.0);
    }
    run_comparison_set("Struct", "Reverse", stu_raw, STRUCT_COUNT, sizeof(Student), cmp_student);

    free(stu_raw);

    printf("\n[Done] Benchmark Completed. Press Enter to exit.\n");
    getchar(); 
    return 0;
}