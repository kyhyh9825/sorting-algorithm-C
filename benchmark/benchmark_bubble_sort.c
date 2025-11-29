#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../library/sorting.h" // Assumes sorting.h is in the same directory

/* --- [Test Configuration] --- */
/* Bubble Sort has O(N^2) time complexity.
 * Performance degrades significantly with large datasets.
 * With 80,000 elements, expect approx. 20-30 seconds on a modern CPU
 * Reduce this number if running on a slower machine. */
#define NUM_ELEMENTS 80000 

/* --- [Test Struct] --- */
/* A heavy structure to simulate realistic data and test memory copy overhead */
typedef struct
{
    long long id;           // Unique ID (8 bytes)
    double probability;     // Probability (8 bytes)
    char name[64];          // Item Name (64 bytes) - Used to test memcpy load
    int rarity;             // Rarity (4 bytes)
} GachaItem;

/* --- [Compare Function] --- */
/* Sort Order: High Rarity -> Low Probability -> Name (Alphabetical) */
int compare_gacha(const void *a, const void *b)
{
    const GachaItem *itemA = (const GachaItem *)a;
    const GachaItem *itemB = (const GachaItem *)b;

    // 1. Rarity Descending (Higher is better)
    if (itemA->rarity != itemB->rarity)
    {
        return itemB->rarity - itemA->rarity; 
    }
    
    // 2. Probability Ascending (Lower is better/rarer)
    if (itemA->probability < itemB->probability) return -1;
    if (itemA->probability > itemB->probability) return 1;

    // 3. Name Ascending (Lexicographical order)
    return strcmp(itemA->name, itemB->name);
}

/* --- [Random String Generator] --- */
void gen_random_string(char *str, int length)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < length - 1; i++)
    {
        str[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    str[length - 1] = '\0';
}

int main(void)
{
    srand((unsigned int)time(NULL));

    printf("==================================================\n");
    printf("   [Benchmark] Bubble Sort Stress Test\n");
    printf("   - Elements: %d\n", NUM_ELEMENTS);
    printf("   - Element Size: %llu bytes (Struct)\n", sizeof(GachaItem));
    printf("   - Estimated Time: 15~40 seconds (depending on hardware)\n");
    printf("==================================================\n\n");

    /* 1. Memory Allocation */
    printf("[1/4] Allocating memory... ");
    GachaItem *items = (GachaItem *)malloc(sizeof(GachaItem) * NUM_ELEMENTS);
    if (items == NULL)
    {
        fprintf(stderr, "\n[Error] Memory allocation failed.\n");
        return 1;
    }
    printf("Done.\n");

    /* 2. Data Generation */
    printf("[2/4] Generating random data... ");
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        items[i].id = i;
        items[i].rarity = rand() % 5 + 1; // 1 to 5 stars
        items[i].probability = (double)rand() / RAND_MAX;
        gen_random_string(items[i].name, 10 + (rand() % 10)); // Name length 10-20
    }
    printf("Done.\n");

    /* 3. Sorting */
    printf("[3/4] Running Bubble Sort (This may take a while)...\n");
    printf("      Processing...");
    fflush(stdout); 

    clock_t start_time = clock();
    
    // Execute Bubble Sort
    bubble_sort(items, NUM_ELEMENTS, sizeof(GachaItem), compare_gacha);

    clock_t end_time = clock();
    double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf(" Finished!\n\n");

    /* 4. Verification */
    printf("[4/4] Verifying sort correctness...\n");
    int sorted_correctly = 1;
    for (int i = 0; i < NUM_ELEMENTS - 1; i++)
    {
        if (compare_gacha(&items[i], &items[i + 1]) > 0)
        {
            sorted_correctly = 0;
            printf("[Failure] Sort order incorrect at index %d.\n", i);
            break;
        }
    }

    if (sorted_correctly)
    {
        printf("\n>>> [Success] Array is sorted correctly.\n");
        printf(">>> Duration: %.4f seconds\n", duration);
    }
    else
    {
        printf("\n>>> [Failure] The algorithm contains bugs.\n");
    }

    free(items);
    printf("\n[Done] Benchmark Completed. Press Enter to exit.\n");
    getchar(); 
    return 0;
}