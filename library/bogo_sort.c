#include <stdio.h>
#include <stdlib.h>

typedef int (*CmpFunc)(const void *a_ptr, const void *b_ptr);

static int is_sorted(void *arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr);
static void shuffle(void *arr, size_t num_of_elements, size_t size_of_element);

void bogo_sort(void *arr ,size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{

}

void bogobogo_sort(void *arr ,size_t num_of_elements, size_t size_of_element, int (*cmp_func_ptr)(const void *a_ptr, const void *b_ptr))
{

}

static int is_sorted(void *arr, size_t num_of_elements, size_t size_of_element, CmpFunc cmp_func_ptr)
{

}

static void shuffle(void *arr, size_t num_of_elements, size_t size_of_element)
{

}

int main(void)
{

    system("pause");
    return 0;
}