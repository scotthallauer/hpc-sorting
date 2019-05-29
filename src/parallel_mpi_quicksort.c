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
        isort(arr, lo, pi-1);
        isort(arr, pi+1, hi);
    }
}

// external sorting function which takes an array and its size, and sorts it using the quicksort algorithm
// (using the isort method).
void quicksort(int arr[], int n)
{
    isort(arr, 0, n-1);
}

// merge different blocks of sorted numbers into a single sorted array
void merge(int *src_arr, int num_blocks, int block_size, int *dest_arr)
{
    int dest_idx = 0;
    int *block_idx;
    block_idx = malloc(sizeof(int) * num_blocks);
    for(int i = 0; i < num_blocks; i++)
        block_idx[i] = 0;
    bool done = false;
    int min_val, min_block;
    do 
    {
        min_val = src_arr[block_idx[0]];
        min_block = 0;
        for(int i = 1; i < num_blocks; i++)
        {
            if(src_arr[block_idx[i]] < min_val)
            {
                min_val = src_arr[block_idx[i]];
                min_block = i;
            }
        }    
        dest_arr[dest_idx++] = src_arr[block_idx[min_block]++];
        for(int i = 0; i < num_blocks; i++)
        {
            if(block_idx[i] >= block_size)
            {
                done = true;
                break;
            }
        }
    }
    while(!done);
    for(int i = 0; i < num_blocks; i++)
        while(block_idx[i] < block_size)
            dest_arr[dest_idx++] = src_arr[block_idx[i]++];
}

// validates that an array of integers is sorted
bool validate(int arr[], int n)
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

int main(int argc, char** argv) {
    double start, finish;
    int n, m;
    int rank, size;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *original_arr = NULL;
    if (rank == ROOT) {
        // Get input data
        FILE *input = fopen("numbers.txt", "r");
        char line[10];
        fgets(line, 10, input);
        sscanf(line, "%d", &n);
        m = n/size;
        original_arr = malloc(n * sizeof(int));
        for(int i = 0; i < n; i++)
        {
            fgets(line, 10, input);
            sscanf(line, "%d", &original_arr[i]);
        }
        start = MPI_Wtime();
    }
    MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&m, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    int *local_arr = (int *)malloc(sizeof(int) * m);

    MPI_Scatter(original_arr, m, MPI_INT, local_arr, m, MPI_INT, ROOT, MPI_COMM_WORLD);

    quicksort(local_arr, m);

    int *sorted_arr = NULL;
    if (rank == ROOT)
        sorted_arr = (int *)malloc(sizeof(int) * n);
    MPI_Gather(local_arr, m, MPI_INT, sorted_arr, m, MPI_INT, ROOT, MPI_COMM_WORLD);
    
    free(local_arr);
    
    if (rank == ROOT)
    {
        free(original_arr);
        int *merged_arr = (int *)malloc(sizeof(int) * n);
        merge(sorted_arr, size, m, merged_arr);
        finish = MPI_Wtime();
        // Output execution time
        if(validate(merged_arr, n))
        {
            printf("Sorted in %f seconds.\n", (finish-start));
        }
        else
        {
            printf("Sorting failed.\n");
        }
  }

  // Clean up
  if (rank == ROOT) {
    free(sorted_arr);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}