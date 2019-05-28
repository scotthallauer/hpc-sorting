/**
 * Parallel Quicksort Algorithm (OpenMP)
 * Scott Hallauer
 * 25/05/2019
 * Adapted code from GeeksforGeeks (https://www.geeksforgeeks.org/cpp-program-for-quicksort/)
 * and OpenMP (https://www.openmp.org/wp-content/uploads/sc16-openmp-booth-tasking-ruud.pdf)
 */
#include <omp.h>
#include <stdio.h>

// swap values for two elements
void swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// take last element in range and place in correct position for sorted array 
// (with smaller values to the left and larger to the right)
int partition(int arr[], int lo, int hi)
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
void isort(int arr[], int lo, int hi)
{
    if(lo < hi)
    {
        int pi = partition(arr, lo, hi);
        if(((pi-1) - lo) > 100){
            #pragma omp task default(none) firstprivate(arr,lo,pi)
            isort(arr, lo, pi-1);
        }else{
            isort(arr, lo, pi-1);
        }
        if((hi - (pi+1)) > 100){
            #pragma omp task default(none) firstprivate(arr,hi,pi)
            isort(arr, pi+1, hi);
        }else{
            isort(arr, pi+1, hi);
        }
        #pragma omp taskwait
    }
}

// external sorting function which takes an array and its size, and sorts it using the quicksort algorithm
// (using the isort method).
void quicksort(int arr[], int n)
{
    #pragma omp parallel default(none) shared(arr,n)
    {
        #pragma omp single nowait
        {
            isort(arr, 0, n-1);
        }
    }
}

// prints out an array of integers
void display(int arr[], int n)
{
    for(int i = 0; i < n; i++){
        printf("%d\n", arr[i]);
    } 
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
    int *arr;
    arr = (int *)malloc(n * sizeof(int));
    for(int i = 0; i < n; i++)
    {
        fgets(line, 10, input);
        sscanf(line, "%d", &arr[i]);
    }
    // Run sorting algorithm
    start = omp_get_wtime();
    quicksort(arr, n);
    finish = omp_get_wtime();
    // Output execution time
    printf("Sorted in %f seconds\n", (finish-start));
}