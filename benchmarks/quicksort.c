// You need to uncomment and comment some blocks in order to benchmark, it is not full automatic.

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

// Inputs
#define ARRAY_SIZE 1000000
#define LOGS_ON 0

void print_array(int *arr)
{
    printf("\n [");
    int i;

    for (i = 0; i < ARRAY_SIZE - 1; ++i)
        printf("%d, ", arr[i]);

    printf("%d]", arr[ARRAY_SIZE - 1]);
}

void swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void quicksort(int *arr, int left, int right)
{
    if (LOGS_ON)
        printf("\n\n WILL QUICKSORT BETWEEN INDEXES [%d, %d]", left, right);

    int i = left, j = right;
    // Partition
    {
        // Pivot is the random one (2)
        //swap(&arr[left], &arr[(((rand() << 15) ^ rand()) % (right - left)) + left]);
        // Pivot is the second lowest one from left, mid and right (3)
        /*
		int mid = (left + right) / 2;
		if((arr[left] < arr[mid] && arr[mid] < arr[right]) ||
		   (arr[right] < arr[mid] && arr[mid] < arr[left])){
		    swap(&arr[left], &arr[mid]);
		}
		else if((arr[mid] < arr[right] && arr[right] < arr[left]) ||
		        (arr[left] < arr[right] && arr[right] < arr[mid])){
		    swap(&arr[left], &arr[right]);
		}
		*/
        // Pivot is the most left by default (1), nothing to do special
        int pivot = arr[left];

        while (i <= j)
        {
            while (arr[i] < pivot)
                i++;

            while (arr[j] > pivot)
                j--;

            if (i <= j)
            {
                swap(&arr[i], &arr[j]);
                i++;
                j--;
            }
        }

        if (LOGS_ON)
        {
            printf("\n After partitioning with pivot value of %d: ", pivot);
            print_array(arr);
        }
    }
    // Recursion
    {
        if (left < j)
            quicksort(arr, left, j);

        if (i < right)
            quicksort(arr, i, right);
    }
}

void benchmark_quicksort(int *arr)
{
    if (LOGS_ON)
    {
        print_array(arr);
    }

    // Measure the time, start
    struct timeval tvBegin, tvEnd, tvDiff;
    gettimeofday(&tvBegin, NULL);
    quicksort(arr, 0, ARRAY_SIZE - 1);
    // Measure the time, end
    gettimeofday(&tvEnd, NULL);
    long int diff = (tvEnd.tv_usec + 1000000 * tvEnd.tv_sec) - (tvBegin.tv_usec + 1000000 * tvBegin.tv_sec);
    tvDiff.tv_sec = diff / 1000000;
    tvDiff.tv_usec = diff % 1000000;
    printf("\n TIME ELAPSED: %ld.%06ld seconds\n", tvDiff.tv_sec, tvDiff.tv_usec);
}

int fill_array(int *arr, int fill_sorted)
{
    printf("\n Filling the array ");
    int i;

    if (fill_sorted)
    {
        printf("with sorted numbers...");

        for (i = 0; i < ARRAY_SIZE; ++i)
            arr[i] = i;
    }
    else
    {
        printf("with random numbers...");

        for (i = 0; i < ARRAY_SIZE; ++i)
            arr[i] = rand();
    }

    printf(" Done!");
}

int main()
{
    printf("\n Array size: %d", ARRAY_SIZE);
    int *numbers = malloc(sizeof(int) * ARRAY_SIZE);
    fill_array(numbers, 1);
    printf("\n >> Running sort on randomly generated numbers.");
    benchmark_quicksort(numbers);
    fill_array(numbers, 1);
    printf("\n >> Running sort on sequential numbers.");
    benchmark_quicksort(numbers);
    free(numbers);
    return 0;
}