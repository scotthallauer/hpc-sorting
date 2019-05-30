/**
 * Parallel Quicksort Algorithm (OpenMP)
 * Scott Hallauer
 * 25/05/2019
 */
#include <omp.h>
#include <stdio.h>
#include <stdbool.h>

static int SERIAL_CUTOFF = 100;

double start, finish;

/********************************************
 *   PARALLEL QUICKSORT ALGORITHM - START   *
 ********************************************/
// Code from GeeksforGeeks (https://www.geeksforgeeks.org/cpp-program-for-quicksort/)
// and Ruud van der Pas (https://www.openmp.org/wp-content/uploads/sc16-openmp-booth-tasking-ruud.pdf)

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
/******************************************
 *   PARALLEL QUICKSORT ALGORITHM - END   *
 ******************************************/

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
        printf("ERROR: Usage parallel_omp_quicksort <input_file>\n");
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