#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>
#include <string.h>

/*** The size of the array to be processed */
unsigned long long int array_size = 0;
unsigned int num_ts = 0;


void swap(float *a, float *b) {
    float t;
    t = *a;
    *a = *b;
    *b = t;
}

void merge_up(float *arr, int n) {
  int step = n / 2;
  while (step > 0) {
    for (int i = 0; i < n; i+=step*2) {
      int j = i;
      for ( int k = 0;k < step; k++) {
        if (arr[j] > arr[j+step]) 
          swap(&arr[j], &arr[j + step]);
        j++;
      }
    }
    step /= 2;
  }
}

void merge_down(float *arr, int n) {
  int step = n / 2;
  while (step > 0) {
    for (int i=0; i < n; i+=step*2) {
      int j = i;
      for (int k = 0;k < step; k++) {
        if (arr[j] < arr[j+step]) 
          swap(&arr[j], &arr[j + step]);
        j++;
      }
    }
    step /= 2;
  }
}


bool isSorted(const float *arr){
  for(int i = 0; i < array_size-1; i++)
    if(arr[i] > arr[i+1]) return false;
  return true;
}


int main(int argc, char ** argv) {
  if(argc != 3) {
    printf("Wrong number of parameters\n");
    return 0;
  }
  
  array_size = (unsigned int) atoi(argv[1]); //array size is a parameter of the user
  num_ts = (unsigned int) atoi(argv[2]); //num of threads is a parameter of the user

  omp_set_num_threads(num_ts);
  omp_set_nested(1);
  float even_data[array_size];
  float odd_data[array_size];

  /* Initialize data in a random way */
  for(int index = 0; index < array_size; index++){
    unsigned int seed = index;
    odd_data[index] = rand_r(&seed) / (double) RAND_MAX;
  }
  int iteration = 0;

  float start = omp_get_wtime();

  // do merges
  for (int s=2; s <= array_size; s *= 2) {
    #pragma omp parallel for num_threads(num_ts) shared(s) schedule(dynamic, 4)
    for (int i=0; i < array_size; i += (s*2)) {
      #pragma omp task
      merge_up(odd_data + i,s);
      merge_down(odd_data + i + s, s);
    }
  }


  float e = omp_get_wtime();
  printf("Time %f\n", e-start);
  const float *final_data = odd_data;

  isSorted(final_data) ? printf("sorted\n") : printf("not sorted\n");
  return 0;
}