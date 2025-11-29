// I used gemini because of my laziness

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h> // 숫자에 쉼표(,) 찍기 위해 사용 (선택 사항)
#include "../library/sorting.h"

// --- 전역 변수: 비교 횟수 카운터 ---
static unsigned long long g_compare_count = 0;

// --- 카운팅 기능이 포함된 비교 함수 ---
int compare_with_count(const void *a, const void *b) {
    g_compare_count++; // 호출될 때마다 카운트 증가
    
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

// 정렬 검증 함수
int verify_sorted(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) return 0;
    }
    return 1;
}

// 랜덤 배열 생성
void generate_random_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100; // 0~99 (보기 편하게 두 자리 수)
    }
}

// 배열 출력 유틸리티
void print_array(const char* label, int *arr, int n) {
    printf("%-7s [ ", label);
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("]\n");
}

// 벤치마크 실행 함수
void run_test(const char *name, void (*sort_func)(void *, size_t, size_t, int (*)(const void *, const void *)), int n) {
    printf("\n==========================================================\n");
    printf(" %s | N = %d\n", name, n);
    printf("==========================================================\n");

    int *arr = (int *)malloc(sizeof(int) * n);
    if (!arr) return;

    generate_random_array(arr, n);
    print_array("Before:", arr, n);

    // 카운터 및 타이머 초기화
    g_compare_count = 0;
    clock_t start = clock();

    // 정렬 실행
    sort_func(arr, n, sizeof(int), compare_with_count);

    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;

    print_array("After:", arr, n);

    // 결과 출력
    if (verify_sorted(arr, n)) {
        printf("Result:  SUCCESS\n");
    } else {
        printf("Result:  FAILED\n");
    }
    
    // 천 단위 쉼표 찍기 (환경에 따라 동작 안 할 수도 있음)
    setlocale(LC_NUMERIC, ""); 
    printf("Time:    %.4f sec\n", elapsed);
    
    // 비교 횟수 출력 (가장 중요한 지표)
    // 윈도우/리눅스 환경에 따라 서식 지정자가 다를 수 있어 안전하게 %llu 사용
    printf("Count:   %llu comparisons\n", g_compare_count);

    free(arr);
}

int main() {
    srand((unsigned int)time(NULL));

    // 1. 일반 보고 정렬 (Bogo Sort)
    // N=12 까지는 현대 컴퓨터로 감당 가능 (수십 초 이내)
    printf("\n>>> Starting Bogo Sort Benchmark (Max N=12) <<<\n");
    int bogo_sizes[] = {5, 8, 10, 12};
    for (int i = 0; i < 4; i++) {
        run_test("Bogo Sort", bogo_sort, bogo_sizes[i]);
    }

    // 2. 보고보고 정렬 (BogoBogo Sort)
    // N=6 은 정말 오래 걸립니다. (컴퓨터 성능에 따라 수 분 ~ 수 시간)
    printf("\n\n>>> Starting BogoBogo Sort Benchmark (Max N=6) <<<\n");
    printf("WARNING: N=6 implies nested factorial recursion. Be patient.\n");
    
    int bogobogo_sizes[] = {4, 5, 6};
    for (int i = 0; i < 3; i++) {
        run_test("BogoBogo Sort", bogobogo_sort, bogobogo_sizes[i]);
    }

    printf("\n[Done] Benchmark completed. Press Enter to exit.\n");
    getchar();
    return 0;
}