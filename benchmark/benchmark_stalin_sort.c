#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../library/sorting.h" // Gulag 구조체 및 stalin_sort 프로토타입 포함

// 정수 비교 함수 (오름차순)
int compare_ints(const void *a, const void *b)
{
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

// 배열 출력 유틸리티
void print_int_array(const char *label, int *arr, size_t size)
{
    printf("%-20s [ ", label);
    for (size_t i = 0; i < size; i++)
    {
        printf("%2d ", arr[i]);
    }
    printf("]\n");
}

int main()
{
    // 1. 초기화
    srand((unsigned int)time(NULL));
    const size_t N = 30;

    printf("=== Stalin Sort Benchmark (N=%zu) ===\n\n", N);

    int *arr = (int *)malloc(sizeof(int) * N);
    if (!arr)
    {
        perror("Failed to allocate memory");
        return 1;
    }

    // 0~99 사이의 랜덤 값 생성
    for (size_t i = 0; i < N; i++)
    {
        arr[i] = rand() % 100;
    }

    // 2. 정렬 전 상태 출력
    print_int_array("Before Purge:", arr, N);
    printf("\nExecution Stalin Sort...\n");

    // 3. 스탈린 정렬 수행
    // 반환된 Gulag 구조체에는 숙청된 원소들이 들어있고,
    // 원본 arr은 압축되어 정렬된 상태(생존자)만 남습니다.
    Gulag *gulag = stalin_sort(arr, N, sizeof(int), compare_ints);

    if (gulag == NULL)
    {
        fprintf(stderr, "Stalin Sort failed (Memory Error?)\n");
        free(arr);
        return 1;
    }

    // 4. 결과 계산
    size_t purged_count = gulag->count;
    size_t survivor_count = N - purged_count;
    int *purged_arr = (int *)gulag->location;

    // 5. 결과 출력
    printf("\n--- Result Report ---\n");

    // 생존자 (원본 배열의 앞부분)
    // 원본 배열은 압축되었으므로, survivor_count만큼만 출력하면 됩니다.
    print_int_array("Survivors (Sorted):", arr, survivor_count);

    // 굴라그 (숙청된 인원)
    if (purged_count > 0)
    {
        print_int_array("Gulag (Purged):", purged_arr, purged_count);
    }
    else
    {
        printf("%-20s [ No one was purged. A perfect society! ]\n", "Gulag (Purged):");
    }

    // 통계
    printf("\nTotal Citizens: %zu\n", N);
    printf("Survivors:      %zu (%.1f%%)\n", survivor_count, (double)survivor_count / N * 100.0);
    printf("Purged:         %zu (%.1f%%)\n", purged_count, (double)purged_count / N * 100.0);

    // 6. 메모리 해제
    // stalin_sort 내부에서 gulag->location을 할당했으므로 반드시 해제해야 함
    if (gulag->location)
        free(gulag->location);
    free(gulag);
    free(arr);

    printf("\n[Done] Benchmark Completed. Press Enter to exit.\n");
    getchar();
    return 0;
}