// I used gemini because of my laziness

/**
 * @file benchmark_unified.c
 * @brief Unified Merge Sort Benchmark
 * * Comparison of Single, Multi, and Multi-PP merge sort algorithms
 * using the SAME dataset size for fairness.
 * * Target duration for Single Thread: ~60 seconds max.
 * Requires 64-bit compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

// Include library header (Relative path)
#include "../library/sorting.h"

// ==========================================
// 1. Data Count Configuration
// ==========================================

// 200 Million elements
// - int (4B): ~800MB raw, ~2.4GB total RAM needed
// - double (8B): ~1.6GB raw, ~4.8GB total RAM needed
// - Expected Single Thread Time: 40s ~ 60s
#define COUNT_PRIMITIVE 200000000LL

// 20 Million elements for Struct
// - Struct (48B): ~1GB raw, ~3GB total RAM needed
// - Copying structs is heavy, so count is reduced to 1/10
#define COUNT_STRUCT     20000000LL

typedef struct
{
    int id;
    char name[32];
    double score;
} Student;

// ==========================================
// 2. Comparison Functions
// ==========================================

int compare_int(const void *a, const void *b)
{
    const int val_a = *(const int *)a;
    const int val_b = *(const int *)b;
    return (val_a > val_b) - (val_a < val_b);
}

int compare_double(const void *a, const void *b)
{
    const double val_a = *(const double *)a;
    const double val_b = *(const double *)b;
    return (val_a > val_b) - (val_a < val_b);
}

int compare_student(const void *a, const void *b)
{
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    // Descending score
    if (sa->score != sb->score)
        return (sb->score > sa->score) - (sb->score < sa->score);
    // Ascending ID
    return (sa->id > sb->id) - (sa->id < sb->id);
}

// ==========================================
// 3. Generators & Validator
// ==========================================

void generate_random_ints(int *arr, size_t n)
{
    printf("   [Info] Generating %lld integers...\n", (long long)n);
    for (size_t i = 0; i < n; i++) arr[i] = rand();
}

void generate_random_doubles(double *arr, size_t n)
{
    printf("   [Info] Generating %lld doubles...\n", (long long)n);
    for (size_t i = 0; i < n; i++) arr[i] = ((double)rand() / RAND_MAX) * 10000.0;
}

void generate_random_students(Student *arr, size_t n)
{
    printf("   [Info] Generating %lld structures...\n", (long long)n);
    for (size_t i = 0; i < n; i++)
    {
        arr[i].id = (int)i;
        arr[i].name[0] = 'S'; arr[i].name[1] = 0; 
        arr[i].score = ((double)rand() / RAND_MAX) * 100.0;
    }
}

int check_sorted(void *arr, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    char *ptr = (char *)arr;
    // Check every element (Strict verification)
    // To save time, you can change loop increment to i+=1000
    for (size_t i = 0; i < n - 1; i++)
    {
        if (cmp(ptr + i * size, ptr + (i + 1) * size) > 0) return 0;
    }
    return 1;
}

// ==========================================
// 4. Benchmark Engine
// ==========================================

typedef void (*SortFunction)(void *arr, size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr));

void measure_and_print(const char *algo_name, SortFunction func, void *original_data, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    // 1. Alloc & Copy
    void *data_copy = malloc(n * size);
    if (!data_copy) {
        printf("| %-25s | %12lld |      OOM       |  FAIL  |\n", algo_name, (long long)n);
        return;
    }
    memcpy(data_copy, original_data, n * size);

    // 2. Measure Time
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    func(data_copy, n, size, cmp);

    QueryPerformanceCounter(&end);
    double elapsed_sec = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

    // 3. Verify (Optional: skip for speed if trusted)
    int sorted = check_sorted(data_copy, n, size, cmp);
    
    // 4. Print Result
    // Fixed width formatting for clean table
    printf("| %-25s | %12lld | %12.4f s | %-6s |\n", 
           algo_name, (long long)n, elapsed_sec, sorted ? "OK" : "FAIL");

    free(data_copy);
}

void run_test_set(const char *type_name, void *data, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    printf("\n");
    printf("==================================================================================\n");
    printf(" Testing: %s \n", type_name);
    printf("==================================================================================\n");
    printf("| Algorithm                 |   Data Count | Execution Time | Status |\n");
    printf("|---------------------------|--------------|----------------|--------|\n");

    // All algorithms sort the SAME amount of data (n)
    measure_and_print("merge_sort (Single)", merge_sort, data, n, size, cmp);
    measure_and_print("merge_sort_multi", merge_sort_multi, data, n, size, cmp);
    measure_and_print("merge_sort_pp", merge_sort_pp, data, n, size, cmp);
    
    printf("----------------------------------------------------------------------------------\n");
}

int main()
{
    srand((unsigned int)time(NULL));

    // System Info
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    
    printf("\n");
    printf("#################################################################\n");
    printf("#            Unified Merge Sort Benchmark (64-bit)              #\n");
    printf("#################################################################\n");
    printf(" System RAM: %.2f GB\n", statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
    printf(" CPU Cores : %d\n", GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
    printf(" ---------------------------------------------------------------\n");
    printf(" [Target Settings]\n");
    printf("  - Primitives (int, double): %lld elements (Expect ~5GB RAM)\n", (long long)COUNT_PRIMITIVE);
    printf("  - Structures (Student)    : %lld elements (Expect ~3GB RAM)\n", (long long)COUNT_STRUCT);
    printf("#################################################################\n");

    // 1. Integer Test
    int *int_arr = (int *)malloc(COUNT_PRIMITIVE * sizeof(int));
    if (int_arr) {
        generate_random_ints(int_arr, COUNT_PRIMITIVE);
        run_test_set("Integer (int)", int_arr, COUNT_PRIMITIVE, sizeof(int), compare_int);
        free(int_arr);
    } else {
        printf("[Error] Failed to allocate memory for Integer test.\n");
    }

    // 2. Double Test
    double *dbl_arr = (double *)malloc(COUNT_PRIMITIVE * sizeof(double));
    if (dbl_arr) {
        generate_random_doubles(dbl_arr, COUNT_PRIMITIVE);
        run_test_set("Double (double)", dbl_arr, COUNT_PRIMITIVE, sizeof(double), compare_double);
        free(dbl_arr);
    } else {
        printf("[Error] Failed to allocate memory for Double test.\n");
    }

    // 3. Structure Test
    Student *stu_arr = (Student *)malloc(COUNT_STRUCT * sizeof(Student));
    if (stu_arr) {
        generate_random_students(stu_arr, COUNT_STRUCT);
        run_test_set("Structure (Student)", stu_arr, COUNT_STRUCT, sizeof(Student), compare_student);
        free(stu_arr);
    } else {
        printf("[Error] Failed to allocate memory for Student test.\n");
    }

    printf("\n[Done] Benchmark Completed. Press Enter to exit.\n");
    getchar(); 
    return 0;
}