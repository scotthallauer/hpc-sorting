/**
 * Regular Sampling Parallel Sort Algorithm (OpenMP)
 * Scott Hallauer
 * 29/05/2019
 * Adapted code from Shane Fitzpatrick (https://github.com/Fitzpasd/Parallel-sort-by-regular-sampling/blob/master/psrs_sort.c)
 */
#include <omp.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double start, finish;

/******************************************
 *   SERIAL QUICKSORT ALGORITHM - START   *
 ******************************************/
// Code from GeeksforGeeks (https://www.geeksforgeeks.org/cpp-program-for-quicksort/)
// Used for validation

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

/****************************
 *    OMP PSRS ALGORITHM    *
 ****************************/
/* headers */
int lcompare(const void * ptr2num1, const void * ptr2num2);
long long *merge(long long * left, long long * right, int l_end, int r_end);
long long *merge_sort(long long * arr, int size);
void insertion_sort(long long *arr, int n);
void calc_partition_borders(long long array[],
               int start,
               int end,
               int sublist_sizes[],
               int at,
               long long pivots[],
               int first_p,
               int last_p);
void psrs_sort(long long *a, int n);
void sortll(long long *a, int len);

/* sort an array in non-descending order */
void psrs_sort(long long *a, int n) {
  if(n > 1){
  if(n <= 55){
	// Testing shows that sequential insertion sort is quickest when n <= 55 (approx.)
	insertion_sort(a,n);
  }else if(n <= 10000){
  // Testing shows that sequential merge sort is quickest when n <= 10000(approx.)
	merge_sort(a,n);
  }else{
  // Testing shows that our algorithm is now the quickest 
  int p, size, rsize, sample_size;
  long long *sample, *pivots;
  int *partition_borders, *bucket_sizes, *result_positions;
  long long **loc_a_ptrs;

  // Determine the appropriate number of threads to use
  // p^3 <= n - We need this to hold true
  p = omp_get_max_threads();
  p = p*p*p;
  if(p > n){
	p = floor(pow(n,0.33));
	p-=p%2;
  }else{
   p = omp_get_max_threads();
   p-=p%2;
  }
  omp_set_num_threads(p);

  size  = (n + p - 1) / p;
  rsize = (size + p - 1) / p;
  sample_size = p * (p - 1);

  loc_a_ptrs = malloc(p * sizeof(long long *));
  sample = malloc(sample_size * sizeof(long long));
  partition_borders = malloc(p * (p + 1) * sizeof(int));
  bucket_sizes = malloc(p * sizeof(int));
  result_positions = malloc(p * sizeof(int));
  pivots = malloc((p - 1) * sizeof(long long));

  #pragma omp parallel
  {
    int i, j, max, thread_num, start, end, loc_size, offset, this_result_size;
    long long *loc_a, *this_result, *current_a;

    thread_num = omp_get_thread_num();
    start = thread_num * size;
    end = start + size - 1;
    if(end >= n) end = n - 1;
    loc_size = (end - start + 1);
    end = end % size;

    loc_a = malloc(loc_size * sizeof(long long));
    memcpy(loc_a, a + start, loc_size * sizeof(long long));
    loc_a_ptrs[thread_num] = loc_a;

    sortll(loc_a, loc_size); // Testing shows that this sequential sort is quickest in this instance
	
    offset = thread_num * (p - 1) - 1;

    for(i = 1; i < p; i++) {
      if(i * rsize <= end) {
        sample[offset + i] = loc_a[i * rsize - 1];
      } else {
        sample[offset + i] = loc_a[end];
      }
    }

    #pragma omp barrier

    #pragma omp single
    {
      merge_sort(sample, sample_size); // Testing shows that this sequential sort is quickest in this instance
      for(i = 0; i < p - 1; i++) {
        pivots[i] = sample[i * p + p / 2];
      }
    }

    #pragma omp barrier

    offset = thread_num * (p + 1);
    partition_borders[offset] = 0;
    partition_borders[offset + p] = end + 1;
    calc_partition_borders(loc_a, 0, loc_size-1, partition_borders, offset, pivots, 1, p-1);

    #pragma omp barrier

    max = p * (p + 1);
    bucket_sizes[thread_num] = 0;
    for(i = thread_num; i < max; i += p + 1) {
      bucket_sizes[thread_num] += partition_borders[i + 1] - partition_borders[i];
    }

    #pragma omp barrier

    #pragma omp single
    {
      result_positions[0] = 0;
      for(i = 1; i < p; i++) {
        result_positions[i] = bucket_sizes[i-1] + result_positions[i-1];
      }
    }

    #pragma omp barrier

    this_result = a + result_positions[thread_num];

    if(thread_num == p-1) {
      this_result_size = n - result_positions[thread_num];
    } else {
      this_result_size = result_positions[thread_num+1] - result_positions[thread_num];
    }

    // pluck this threads sublist from each of the local arrays
    this_result = a + result_positions[thread_num];

    for(i = 0, j = 0; i < p; i++) {
      int low, high, partition_size;
      offset = i * (p + 1) + thread_num;
      low = partition_borders[offset];
      high = partition_borders[offset+1];
      partition_size = (high - low);
      if(partition_size > 0) {
        memcpy(this_result+j, &(loc_a_ptrs[i][low]), partition_size * sizeof(long long));
        j += partition_size;
      }
    }

    // sort p local sorted arrays
    sortll(this_result, this_result_size); // Testing shows that this sequential sort is quickest in this instance
	
    #pragma omp barrier
    free(loc_a);
  }

  free(loc_a_ptrs);
  free(sample);
  free(partition_borders);
  free(bucket_sizes);
  free(result_positions);
  free(pivots);
  }
  }
}


/* determine the boundaries for the sublists of an local array */
void calc_partition_borders(long long array[],    // array being sorted
                            int start,
                            int end,              // separate the array into current process range
                            int result[],
                            int at,               // this process start point in result
                            long long pivots[],   // the pivot values
                            int first_pv,         // first pivot
                            int last_pv)          // last pivot
{
  int mid, lowerbound, upperbound, center;
  long long pv;

  mid = (first_pv + last_pv) / 2;
  pv = pivots[mid-1];
  lowerbound = start;
  upperbound = end;
  while(lowerbound <= upperbound) {
    center = (lowerbound + upperbound) / 2;
    if(array[center] > pv) {
      upperbound = center - 1;
    } else {
      lowerbound = center + 1;
    }
  }
  result[at + mid] = lowerbound;

  if(first_pv < mid) {
    calc_partition_borders(array, start, lowerbound - 1, result, at, pivots, first_pv, mid - 1);
  }
  if(mid < last_pv) {
    calc_partition_borders(array, lowerbound, end, result, at, pivots, mid + 1, last_pv);
  }
}


/* Compare long longs (lifted from the harness) */
int lcompare(const void * ptr2num1, const void * ptr2num2)
{
  long long num1 = *((long long*) ptr2num1);
  long long num2 = *((long long*) ptr2num2);

  if ( num1 > num2 )
    return 1;
  else if ( num1 < num2 )
    return -1;
  else
    return 0;
}


/*
  Sort a portion of an array
  @todo: see if merge sort might be better in some cases
*/
void sortll(long long *a, int len)
{
  qsort(a, len, sizeof(long long), lcompare);
}

/*
  Standard merge sort
*/
long long *merge_sort(long long * arr, int size){
	// Arrays shorter than 1 are already sorted
	if(size > 1){
		int middle = size / 2, i; 
		long long *left, *right;
		left = arr;
		right = arr + middle; 
		
		left = merge_sort(left, middle);
		right = merge_sort(right, size-middle);
		return merge(left, right, middle,size-middle);
	}else { return arr; }
}

long long *merge(long long * left, long long * right, int l_end, int r_end){
	int temp_off, l_off, r_off, size = l_end+r_end;
	long long *temp = malloc(sizeof(long long) * l_end);

	// Copy lower half into temp buffer
	for(l_off=0, temp_off=0; left+l_off != right; l_off++, temp_off++){
		*(temp + temp_off) = *(left + l_off);
	}
	
	temp_off=0; l_off=0; r_off=0;

	while(l_off < size){
		if(temp_off < l_end){
			if(r_off < r_end){
				if(*(temp+temp_off) < *(right+r_off)){
					*(left+l_off) = *(temp+temp_off);
					temp_off++;
				}else{
					*(left+l_off) = *(right+r_off);
					r_off++;
				}
			}else{
				*(left+l_off) = *(temp+temp_off);
				temp_off++;
			}
		}else{
			if(r_off < r_end) {
				*(left + l_off) = *(right + r_off);
				r_off++;
			}else{
				printf("\nERROR - merging loop going too far\n");
			}
		}
		l_off++;
	}
	free(temp);
	return left;
}

/*
  Standard insertion sort
*/
void insertion_sort(long long *arr, int n){
	int i, j, k, temp;
	
	for ( i = 1 ; i <= n ; i++ )
	{
		for ( j = 0 ; j < i ; j++ )
		{
			if ( arr[j] > arr[i] )
			{
				temp = arr[j] ;
				arr[j] = arr[i] ;

				for ( k = i ; k > j ; k-- )
					arr[k] = arr[k - 1] ;

				arr[k + 1] = temp ;
			}
		}
	}
}

int main(int argc, char **argv)
{
    // CHECK ARUGMENTS //
    if(argc != 2 || argv[1] == NULL)
    {
        printf("ERROR: Usage parallel_omp_regsampling <input_file>\n");
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
    long long *original_arr;
    original_arr = malloc(n * sizeof(long long));
    for(int i = 0; i < n; i++)
      original_arr[i] = arr[i];

    // RUN SORTING ALGORITHM //
    start = omp_get_wtime();
    psrs_sort(arr, n);
    finish = omp_get_wtime();

    // OUTPUT EXECUTION TIME //
    if(validate(original_arr, arr, n))
        printf("%f\n", (finish-start));

    // CLEAN UP //
    free(original_arr);
    free(arr);

    return 0;
}