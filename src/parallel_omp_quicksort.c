/**
 * Parallel Quicksort Algorithm (OpenMP)
 * Scott Hallauer
 * 25/05/2019
 * Adapted code from GeeksforGeeks (https://www.geeksforgeeks.org/cpp-program-for-quicksort/)
 * and OpenMP (https://www.openmp.org/wp-content/uploads/sc16-openmp-booth-tasking-ruud.pdf)
 */
#include <omp.h>
#include <stdio.h>
#include <stdbool.h>

static int SERIAL_CUTOFF = 100;

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
        if(((pi-1) - lo) > SERIAL_CUTOFF)
        {
            #pragma omp task default(none) firstprivate(arr,lo,pi)
            isort(arr, lo, pi-1);
        }
        else
        {
            isort(arr, lo, pi-1);
        }
        if((hi - (pi+1)) > SERIAL_CUTOFF)
        {
            #pragma omp task default(none) firstprivate(arr,hi,pi)
            isort(arr, pi+1, hi);
        }
        else
        {
            isort(arr, pi+1, hi);
        }
        #pragma omp taskwait
    }
}

// external sorting function which takes an array and its size, and sorts it using the quicksort algorithm
// (using the isort method).
void quicksort(long long *arr, int n)
{
    #pragma omp parallel default(none) shared(arr,n)
    {
        #pragma omp single nowait
        {
            isort(arr, 0, n-1);
        }
    }
}

// validates that an array of integers is sorted
bool validate(long long *arr, int n)
{
    for(int i = 0; i < n-1; i++)
    {
        if(arr[i] > arr[i+1])
        {
            return false;
        }
    } 
    return true;
}

int main()
{
    // Get input data
    double start, finish;
    FILE *input = fopen("numbers.txt", "r");
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
    // Run sorting algorithm
    start = omp_get_wtime();
    quicksort(arr, n);
    finish = omp_get_wtime();
    // Output execution time
    if(validate(arr, n))
    {
        printf("Sorted in %f seconds.\n", (finish-start));
    }
    else
    {
        printf("Sorting failed.\n");
    }
    return 0;
}