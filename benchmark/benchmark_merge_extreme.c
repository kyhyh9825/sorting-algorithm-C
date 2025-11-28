// I used gemini because of my laziness

/**
 * @file benchmark_optimal.c
 * @brief Optimal Merge Sort Benchmark for 32GB RAM Systems
 * * Target: ~500 Million elements (Approx. 4GB Data + 4GB Buffer = 8GB Total)
 * * Estimated Duration: < 5 minutes total
 * * Compares: Single vs Multi vs Ping-Pong
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <stdint.h>

// 라이브러리 헤더 포함
#include "../library/sorting.h"

// ==========================================
// 1. 설정 (5억 개)
// ==========================================
// 500,000,000 doubles * 8 bytes = ~3.72 GB
// Double buffering requires ~7.5 GB Total RAM.
// Ideally fits in 16GB+ RAM systems without swapping.
#define COUNT_OPTIMAL 500000000LL
#define RANDOM_SEED 9999ULL

// ==========================================
// 2. 고속 난수 생성기 (Xorshift)
// ==========================================
static uint64_t xorshift64(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return *state = x;
}

void fill_random_doubles(double *arr, size_t n, uint64_t seed) {
    printf("   [Init] Generating %lld random elements... ", (long long)n);
    uint64_t state = seed;
    // OMP Parallel can be used here if available, but serial is fast enough for 500M
    for (size_t i = 0; i < n; i++) {
        uint64_t r = xorshift64(&state);
        arr[i] = (r >> 11) * (1.0 / 9007199254740992.0); 
    }
    printf("Done.\n");
}

// ==========================================
// 3. 비교 및 검증
// ==========================================
int compare_double(const void *a, const void *b) {
    const double val_a = *(const double *)a;
    const double val_b = *(const double *)b;
    return (val_a > val_b) - (val_a < val_b);
}

int verify_sorted_sample(double *arr, size_t n) {
    printf("   [Verify] Checking integrity... ");
    size_t step = 100000; // Check every 100,000th element
    for (size_t i = 0; i < n - 1; i += step) {
        if (arr[i] > arr[i+1] && i+1 < n) {
             if (compare_double(&arr[i], &arr[i+1]) > 0) return 0;
        }
    }
    printf("Pass.\n");
    return 1;
}

// ==========================================
// 4. 벤치마크 엔진
// ==========================================
typedef void (*SortFunction)(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

void run_test(const char *algo_name, SortFunction func, size_t count) {
    printf("\n");
    printf("=================================================================\n");
    printf(" Algorithm: %s\n", algo_name);
    printf("=================================================================\n");

    // 1. 메모리 할당
    size_t data_size = count * sizeof(double);
    printf("   [Alloc] Allocating %.2f GB... ", data_size / (1024.0*1024.0*1024.0));
    
    double *data = (double *)malloc(data_size);
    if (!data) {
        printf("FAILED (Out of Memory)\n");
        return;
    }
    printf("Success.\n");

    // 2. 데이터 생성 (동일 시드)
    fill_random_doubles(data, count, RANDOM_SEED);

    // 3. 실행 및 시간 측정
    printf("   [Sort] Running sort...\n");
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    func(data, count, sizeof(double), compare_double);

    QueryPerformanceCounter(&end);
    double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;

    // 4. 결과 출력
    int valid = verify_sorted_sample(data, count);
    printf("   -------------------------------------------------------------\n");
    printf("   >> Time  : %.4f seconds\n", elapsed);
    printf("   >> Status: %s\n", valid ? "OK" : "FAIL");

    // 5. 메모리 즉시 해제 (중요: 다음 테스트를 위해 RAM 비우기)
    free(data);
    printf("   [Free] Memory released.\n");
}

int main() {
    // 시스템 사양 확인
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    printf("\n");
    printf("#################################################################\n");
    printf("#            EXTREME MERGE SORT BENCHMARK (500M)                #\n");
    printf("#################################################################\n");
    printf(" System RAM : %.2f GB\n", statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
    printf(" Target Data: %lld doubles (approx. 4GB Data + 4GB Buffer)\n", (long long)COUNT_OPTIMAL);
    printf("#################################################################\n");

    // 1. Single Thread (기준점)
    // 5억개 정렬 시 약 1~3분 소요 예상
    run_test("merge_sort (Single Thread)", merge_sort, COUNT_OPTIMAL);
    
    Sleep(1000); // 메모리 정리 대기

    // 2. Multi Thread (Standard)
    run_test("merge_sort_multi (Standard Parallel)", merge_sort_multi, COUNT_OPTIMAL);

    Sleep(1000);

    // 3. Multi Thread (Ping-Pong)
    run_test("merge_sort_pp (Ping-Pong Optimized)", merge_sort_pp, COUNT_OPTIMAL);

    printf("\n[Done] Benchmark completed. Press Enter to exit.\n");
    getchar();
    return 0;
}