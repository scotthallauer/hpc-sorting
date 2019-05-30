/**
 * Serial Recursive Quicksort Algorithm 
 * Scott Hallauer
 * 25/05/2019
 */
#include <omp.h>
#include <stdio.h>
#include <stdbool.h>

double start, finish;

/******************************************
 *   SERIAL QUICKSORT ALGORITHM - START   *
 ******************************************/
// Code from GeeksforGeeks (https://www.geeksforgeeks.org/cpp-program-for-quicksort/)

// swap values for two elements
void swap(long long *a, long long *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// take last element in range and place in correct position for sorted array 
// (with smaller values to the left and larger to the right)
int partition(long long *arr, int lo, int hi)
{
    int pivot = arr[hi];
    int i = (lo-1);

    for(int j = lo; j <= hi-1; j++)
    {
        if(arr[j] <= pivot)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }

    swap(&arr[i+1], &arr[hi]);
    return (i+1);
}

// internal sorting function which implements the quicksort algorithm
// (i.e. partition and then sort left and right sides)
void isort(long long *arr, int lo, int hi)
{
    if(lo < hi)
    {
        int pi = partition(arr, lo, hi);
        isort(arr, lo, pi-1);
        isort(arr, pi+1, hi);
    }
}

// external sorting function which takes an array and its size, and sorts it using the quicksort algorithm
// (using the isort method).
void quicksort(long long *arr, int n)
{
    isort(arr, 0, n-1);
}
/****************************************
 *   SERIAL QUICKSORT ALGORITHM - END   *
 ****************************************/

// validates that an array of integers is sorted correctly
bool validate(long long *arr, int n)
{
    for(int i = 0; i < n-1; i++)
    {
        if(arr[i] > arr[i+1])
        {
            printf("ERROR: Validation failed at element %d\n", i);
            return false;
        }
    } 
    return true;
}

int main(int argc, char **argv)
{
    // CHECK ARUGMENTS //
    if(argc != 2 || argv[1] == NULL)
    {
        printf("ERROR: Usage serial_quicksort <input_file>\n");
        exit(1);
    }

    // READ INPUT DATA //
    FILE *input = fopen(argv[1], "r");
    char line[10];
    int n;
    fgets(line, 10, input);
    sscanf(line, "%d", &n);
    long long *arr;
    arr = malloc(n * sizeof(long long));
    for(int i = 0; i < n; i++)
    {
        fgets(line, 10, input);
        sscanf(line, "%lld", &arr[i]);
    }

    // RUN SORTING ALGORITHM //
    start = omp_get_wtime();
    quicksort(arr, n);
    finish = omp_get_wtime();

    // OUTPUT EXECUTION TIME //
    if(validate(arr, n))
        printf("%f\n", (finish-start));

    // CLEAN UP //
    free(arr);
    
    return 0;
}