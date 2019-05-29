/**
 * Parallel Quicksort Algorithm (MPI)
 * Scott Hallauer
 * 28/05/2019
 * Adapted code from GeeksforGeeks (https://www.geeksforgeeks.org/cpp-program-for-quicksort/),
 * GeeksforGeeks (https://www.geeksforgeeks.org/merge-two-sorted-arrays/)
 * and eerpini (https://github.com/eerpini/Quick-Sort-MPI/blob/master/mpi_pqsort.c)
 */
#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

static int ROOT = 0;
static int MSG_COUNT = 0;
static int MSG_ARRAY = 1;

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

void divide(int arr[], int lo, int hi, int num_ranks, int start_rank){
    if(num_ranks == 1)
    {
        int count = hi-lo+1;
        MPI_Send(&count, 1, MPI_INT, start_rank, MSG_COUNT, MPI_COMM_WORLD);
        MPI_Send(arr + lo, count, MPI_INT, start_rank, MSG_ARRAY, MPI_COMM_WORLD);
    }
    else
    {
        int l_num_ranks = num_ranks/2;
        int r_num_ranks = l_num_ranks;
        if(num_ranks % 2 != 0)
            r_num_ranks = l_num_ranks + 1;
        int r_start_rank = start_rank + l_num_ranks;
        if(lo < hi)
        {
            int pi = partition(arr, lo, hi);
            divide(arr, lo, pi, l_num_ranks, start_rank);
            divide(arr, pi+1, hi, r_num_ranks, r_start_rank);
        }
    }
}

void merge(int arr1[], int n1, int arr2[], int n2, int arr3[])
{
    int i1 = 0, i2 = 0, i3 = 0;
    while(i1 < n1 && i2 < n2)
    {
        if(arr1[i1] < arr2[i2])
            arr3[i3++] = arr1[i1++];
        else
            arr3[i3++] = arr2[i2++];
    }
    while(i1 < n1)
        arr3[i3++] = arr1[i1++];
    while(i2 < n2)
        arr3[i3++] = arr2[i2++];
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

int main(int argc, char *argv[])
{
    double start, finish;
    int rank, size, n;
    int *original_arr;
    int *local_arr;
    int *sorted_arr;
    int *arr1; 
    int *arr2;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if(rank == ROOT)
    {
        // Get input data
        FILE *input = fopen("numbers.txt", "r");
        char line[10];
        fgets(line, 10, input);
        sscanf(line, "%d", &n);
        original_arr = malloc(n * sizeof(int));
        for(int i = 0; i < n; i++)
        {
            fgets(line, 10, input);
            sscanf(line, "%d", &original_arr[i]);
        }
        start = MPI_Wtime();
        //divide(arr, 0, n-1, size-1, 1);
    }
    /*
    else
    {
        int n;
        MPI_Status status;
        MPI_Recv(&n, 1, MPI_INT, MASTER, MSG_COUNT, MPI_COMM_WORLD, &status);
        printf("Process %d is receiving %d items.\n", rank, n);
        int *arr2;
        arr2 = malloc(n * sizeof(int));
        MPI_Recv(arr2, n, MPI_INT, MASTER, MSG_ARRAY, MPI_COMM_WORLD, &status);
        quicksort(arr2, n);
    }
    */
    int m = n/size;
    local_arr = malloc(m * sizeof(int));
    MPI_Scatter(original_arr, m, MPI_INT, local_arr, m, MPI_INT, ROOT, MPI_COMM_WORLD);
    quicksort(local_arr, m);
    MPI_Request *request;
    request = malloc(size * sizeof(MPI_Request));
    MPI_Isend(local_arr, m, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &request[rank]);

    if(rank == ROOT)
    {
        MPI_Status status;
        sorted_arr = malloc(m * sizeof(int));
        MPI_Recv(sorted_arr, m, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &status);
        int received = 1;
        //int *flag;
        //flag = malloc(size * sizeof(int));
        int flag;
        bool *done;
        done = malloc(size * sizeof(bool));
        done[0] = true;
        for(int i = 1; i < size; i++)
        {
            done[i] = false;
        }
        while(received < 2)
        {
            for(int i = 0; i < size; i++)
            {
                if(!done[i])
                {
                    MPI_Test(&request[i], &flag, &status);
                    //if(flag[i])
                    if(flag)
                    {
                        //arr2 = malloc(m * sizeof(int));
                        MPI_Recv(arr2, m, MPI_INT, ROOT, i, MPI_COMM_WORLD, &status);
                        //done[i] = true;
                        received++;
                    }
                }
            }
        }/*
        for(int i = 1; i < size; i++)
        {
            arr1 = sorted_arr;
            arr2 = malloc(m * sizeof(int));
            MPI_Recv(arr2, m, MPI_INT, ROOT, i, MPI_COMM_WORLD, &status);
            sorted_arr = malloc((i+1) * m * sizeof(int));
            merge(arr1, m, arr2, m, sorted_arr);
        }*/
        finish = MPI_Wtime();
        printf("Sorted in %f seconds\n", (finish-start));
    }
    MPI_Finalize();
    return 0;
}