/**
 * @file benchmark_heap_types.c
 * @brief Heap Sort Benchmark for Multiple Data Types
 * * Fixed: Added CmpFunc typedef locally
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <stdint.h>

#include "../library/sorting.h"

// ==========================================
// 0. Type Definitions
// ==========================================

// Define CmpFunc locally since it is not in sorting.h
typedef int (*CmpFunc)(const void *a, const void *b);

// ==========================================
// 1. Configuration
// ==========================================
#define COUNT_INT    50000000LL  // 50 Million ints
#define COUNT_DBL    50000000LL  // 50 Million doubles
#define COUNT_STRUCT 10000000LL  // 10 Million structs (approx 1.2 GiB)

#define RANDOM_SEED 12345ULL

// ==========================================
// 2. Data Type Definitions
// ==========================================

// [Type 3] Large Struct (128 bytes)
// Designed to test memcpy/memmove performance overhead
typedef struct {
    int64_t key;        // 8 bytes (Sorting Key)
    char padding[120];  // 120 bytes (Junk Data)
} HeavyStruct;

// ==========================================
// 3. Random Generator (Xorshift)
// ==========================================
static uint64_t xorshift64(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return *state = x;
}

// ==========================================
// 4. Comparison Functions
// ==========================================

int cmp_int(const void *a, const void *b) {
    const int val_a = *(const int *)a;
    const int val_b = *(const int *)b;
    return (val_a > val_b) - (val_a < val_b);
}

int cmp_dbl(const void *a, const void *b) {
    const double val_a = *(const double *)a;
    const double val_b = *(const double *)b;
    return (val_a > val_b) - (val_a < val_b);
}

int cmp_struct(const void *a, const void *b) {
    const HeavyStruct *sa = (const HeavyStruct *)a;
    const HeavyStruct *sb = (const HeavyStruct *)b;
    if (sa->key > sb->key) return 1;
    if (sa->key < sb->key) return -1;
    return 0;
}

// ==========================================
// 5. Data Generators
// ==========================================

void gen_int(void *arr, size_t n, uint64_t seed) {
    int *data = (int *)arr;
    uint64_t state = seed;
    for (size_t i = 0; i < n; i++) {
        data[i] = (int)(xorshift64(&state) % 2147483647);
    }
}

void gen_dbl(void *arr, size_t n, uint64_t seed) {
    double *data = (double *)arr;
    uint64_t state = seed;
    for (size_t i = 0; i < n; i++) {
        uint64_t r = xorshift64(&state);
        data[i] = (r >> 11) * (1.0 / 9007199254740992.0);
    }
}

void gen_struct(void *arr, size_t n, uint64_t seed) {
    HeavyStruct *data = (HeavyStruct *)arr;
    uint64_t state = seed;
    for (size_t i = 0; i < n; i++) {
        data[i].key = (int64_t)xorshift64(&state);
        // Only key matters for sorting, padding can be garbage
    }
}

// ==========================================
// 6. Verifiers
// ==========================================

int verify_int(void *arr, size_t n) {
    int *data = (int *)arr;
    size_t step = 50000;
    for (size_t i = 0; i < n - 1; i += step) {
        if (data[i] > data[i+1] && i+1 < n) {
             if (cmp_int(&data[i], &data[i+1]) > 0) return 0;
        }
    }
    return 1;
}

int verify_dbl(void *arr, size_t n) {
    double *data = (double *)arr;
    size_t step = 50000;
    for (size_t i = 0; i < n - 1; i += step) {
        if (data[i] > data[i+1] && i+1 < n) {
             if (cmp_dbl(&data[i], &data[i+1]) > 0) return 0;
        }
    }
    return 1;
}

int verify_struct(void *arr, size_t n) {
    HeavyStruct *data = (HeavyStruct *)arr;
    size_t step = 10000;
    for (size_t i = 0; i < n - 1; i += step) {
        if (data[i].key > data[i+1].key && i+1 < n) {
             if (cmp_struct(&data[i], &data[i+1]) > 0) return 0;
        }
    }
    return 1;
}

// ==========================================
// 7. Generic Benchmark Engine
// ==========================================

typedef void (*GenFunc)(void *arr, size_t n, uint64_t seed);
typedef int (*VerifyFunc)(void *arr, size_t n);

void run_benchmark(
    const char *label, 
    size_t count, 
    size_t size, 
    CmpFunc cmp, 
    GenFunc gen, 
    VerifyFunc verify
) {
    printf("\n");
    printf("=================================================================\n");
    printf(" Target: %s\n", label);
    printf(" Items : %lld\n", (long long)count);
    printf(" Size  : %lld bytes / element\n", (long long)size);
    printf("=================================================================\n");

    // 1. Alloc
    size_t total_bytes = count * size;
    printf("   [Alloc] %.2f MiB... ", total_bytes / (1024.0 * 1024.0));
    void *data = malloc(total_bytes);
    if (!data) {
        printf("FAILED (Out of Memory)\n");
        return;
    }
    printf("OK.\n");

    // 2. Gen
    printf("   [Gen]   Generating random data... ");
    gen(data, count, RANDOM_SEED);
    printf("OK.\n");

    // 3. Sort
    printf("   [Sort]  Running heap_sort... ");
    
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    // Casting cmp to match the exact signature expected by heap_sort if necessary,
    // but usually CmpFunc (int (*)(const void*, const void*)) is compatible.
    heap_sort(data, count, size, cmp);

    QueryPerformanceCounter(&end);
    double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    printf("Done.\n");

    // 4. Verify
    printf("   [Check] Verifying order... ");
    int valid = verify(data, count);
    printf("%s\n", valid ? "PASS" : "FAIL");

    // 5. Result
    printf("   -------------------------------------------------------------\n");
    printf("   >> Time: %.4f sec\n", elapsed);
    
    // 6. Free
    free(data);
}

int main() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    printf("\n");
    printf("#################################################################\n");
    printf("#              MULTI-TYPE HEAP SORT BENCHMARK                   #\n");
    printf("#################################################################\n");
    printf(" System RAM: %.2f GiB\n", statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
    printf("#################################################################\n");

    // Test 1: Integer
    run_benchmark("Integer (int)", COUNT_INT, sizeof(int), cmp_int, gen_int, verify_int);

    // Test 2: Double
    run_benchmark("Float (double)", COUNT_DBL, sizeof(double), cmp_dbl, gen_dbl, verify_dbl);

    // Test 3: Large Struct
    run_benchmark("Struct (128 bytes)", COUNT_STRUCT, sizeof(HeavyStruct), cmp_struct, gen_struct, verify_struct);

    printf("\n[All Tests Completed] Press Enter to exit.\n");
    getchar();
    return 0;
}