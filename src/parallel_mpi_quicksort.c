/**
 * Parallel Quicksort Algorithm (MPI)
 * Scott Hallauer
 * 29/05/2019
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mpi.h>

static int ROOT = 0;

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

// merge different blocks of sorted numbers into a single sorted array
void merge(long long *src_arr, int num_blocks, int block_size, long long *dest_arr)
{
    int dest_idx = 0;
    int *block_idx;
    block_idx = malloc(sizeof(int) * num_blocks);
    for(int i = 0; i < num_blocks; i++)
        block_idx[i] = (i * block_size);
    int min_val, min_block;
    do 
    {
        min_val = __INT_MAX__;
        min_block = 0;
        for(int i = 0; i < num_blocks; i++)
        {
            if(block_idx[i] < ((i+1) * block_size) && src_arr[block_idx[i]] < min_val)
            {
                min_val = src_arr[block_idx[i]];
                min_block = i;
            }
        }    
        dest_arr[dest_idx++] = src_arr[block_idx[min_block]++];
    }
    while(dest_idx < (num_blocks * block_size));
}

// validates that an array of integers is sorted correctly
bool validate(long long *original_arr, long long *final_arr, int n)
{
    quicksort(original_arr, n);
    for(int i = 0; i < n; i++)
    {
        if(original_arr[i] != final_arr[i])
        {
            printf("ERROR: Validation failed at element %d - expected %lld but found %lld\n", i, original_arr[i], final_arr[i]);
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
        printf("ERROR: Usage parallel_mpi_quicksort <input_file>\n");
        exit(1);
    }

    int n, m;
    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long *original_arr = NULL;

    if (rank == ROOT) {
        // READ INPUT DATA //
        FILE *input = fopen(argv[1], "r");
        char line[10];
        fgets(line, 10, input);
        sscanf(line, "%d", &n);
        m = n/size;
        original_arr = malloc(n * sizeof(long long));
        for(int i = 0; i < n; i++)
        {
            fgets(line, 10, input);
            sscanf(line, "%lld", &original_arr[i]);
        }

        // START TIMING //
        start = MPI_Wtime();
    }

    MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&m, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    long long *local_arr;
    local_arr = malloc(m * sizeof(long long));

    MPI_Scatter(original_arr, m, MPI_LONG_LONG, local_arr, m, MPI_LONG_LONG, ROOT, MPI_COMM_WORLD);

    quicksort(local_arr, m);

    long long *sorted_arr = NULL;

    if (rank == ROOT)
        sorted_arr = malloc(n * sizeof(long long));

    MPI_Gather(local_arr, m, MPI_LONG_LONG, sorted_arr, m, MPI_LONG_LONG, ROOT, MPI_COMM_WORLD);
    
    if (rank == ROOT)
    {
        long long *merged_arr;
        merged_arr = malloc(n * sizeof(long long));
        merge(sorted_arr, size, m, merged_arr);

        // STOP TIMING //
        finish = MPI_Wtime();

        // OUTPUT EXECUTION TIME //
        if(validate(original_arr, merged_arr, n))
            printf("%f\n", (finish-start));
    }

    // CLEAN UP //
    if (rank == ROOT)
    {
        free(original_arr);
        free(sorted_arr);
    }
    free(local_arr);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}