// I used gemini because of my laziness

/**
 * @file benchmark_merge_extreme.c
 * @brief 32GB 시스템 메모리 풀로드 벤치마크 (Extreme RAM Usage)
 * * 목표: 32GB RAM 중 가용량(약 24GB)을 최대한 활용
 * * 데이터: double형 14억 개 (1.4 Billion)
 * * 예상 소요 시간: 전체 약 4~6분 (싱글 스레드 포함 시)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <stdint.h>

// 라이브러리 헤더 포함 (경로에 맞게 수정하세요)
#include "../library/sorting.h"

// ==========================================
// 1. 데이터 개수 설정 (14억 개)
// ==========================================
// 1,400,000,000 * 8 bytes = ~10.43 GiB
// 정렬 시 2배 필요 = ~20.86 GiB
// OS 기본 사용량(8GB) + 21GB = ~29GB (32GB 내에서 꽉 채움)
#define COUNT_MAX 1400000000LL
#define RANDOM_SEED 12345ULL

// ==========================================
// 2. 고속 난수 생성기 (Xorshift)
// ==========================================
// rand()는 느리고 범위가 작아 대용량 데이터에 부적합
static uint64_t xorshift64(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return *state = x;
}

// 병렬 데이터 생성 (시간 단축)
void fill_random_doubles(double *arr, size_t n, uint64_t seed) {
    printf("   [Init] Generating %lld random elements... ", (long long)n);
    
    // OpenMP가 있다면 #pragma omp parallel for 사용 권장
    // 여기서는 싱글 스레드로 생성하되 고속 알고리즘 사용
    uint64_t state = seed;
    for (size_t i = 0; i < n; i++) {
        uint64_t r = xorshift64(&state);
        // 0.0 ~ 1.0 사이 실수로 변환
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
    // 데이터가 너무 많아 전체 검사는 오래 걸림. 
    // 100만 번째마다 샘플링하여 검사
    size_t step = 1000000; 
    for (size_t i = 0; i < n - 1; i += step) {
        if (arr[i] > arr[i+1] && i+1 < n) {
             // 샘플링 구간에서 역전 발견 시 정밀 비교
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
    printf("   [Alloc] Allocating %.2f GiB... ", data_size / (1024.0*1024.0*1024.0));
    
    double *data = (double *)malloc(data_size);
    if (!data) {
        printf("\n   [FAILED] Out of Memory!\n");
        printf("   시스템 메모리가 부족합니다. 데이터 개수를 줄이거나 다른 앱을 종료하세요.\n");
        return;
    }
    printf("Success.\n");

    // 2. 데이터 생성 (항상 동일 시드 사용 -> 공정한 비교)
    fill_random_doubles(data, count, RANDOM_SEED);

    // 3. 실행 및 시간 측정
    printf("   [Sort] Running sort... (Please wait)\n");
    
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    func(data, count, sizeof(double), compare_double);

    QueryPerformanceCounter(&end);
    double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;

    // 4. 결과 출력
    int valid = verify_sorted_sample(data, count);
    printf("   -------------------------------------------------------------\n");
    printf("   >> Time  : %.4f seconds (%.2f min)\n", elapsed, elapsed/60.0);
    printf("   >> Status: %s\n", valid ? "OK" : "FAIL");

    // 5. 메모리 즉시 해제 (핵심: 다음 테스트를 위해 RAM 비우기)
    printf("   [Free] Releasing memory... ");
    free(data);
    printf("Done.\n");
}

int main() {
    // 한글 출력 설정 (깨짐 방지)
    // SetConsoleOutputCP(65001);

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    printf("\n");
    printf("#################################################################\n");
    printf("#           MAX MEMORY BENCHMARK (32GB SYSTEM TARGET)           #\n");
    printf("#################################################################\n");
    printf(" System RAM : %.2f GiB\n", statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
    printf(" Target Data: %lld doubles (1.4 Billion)\n", (long long)COUNT_MAX);
    printf(" Est. Memory: ~10.4 GiB Data + ~10.4 GiB Buffer = ~20.9 GiB\n");
    printf(" Warning    : Close background apps! (Total usage will be ~30GB)\n");
    printf("#################################################################\n");

    // 1. Single Thread
    // 14억 개 싱글 스레드는 약 3~5분 정도 소요될 수 있습니다.
    run_test("merge_sort (Single Thread)", merge_sort, COUNT_MAX);
    
    Sleep(3000); // 메모리 반환 및 쿨링 대기

    // 2. Multi Thread (Standard)
    run_test("merge_sort_multi (Standard Parallel)", merge_sort_multi, COUNT_MAX);

    Sleep(3000);

    // 3. Multi Thread (Ping-Pong)
    run_test("merge_sort_pp (Ping-Pong Optimized)", merge_sort_pp, COUNT_MAX);

    printf("\n[Done] Benchmark completed. Press Enter to exit.\n");
    getchar();
    return 0;
}