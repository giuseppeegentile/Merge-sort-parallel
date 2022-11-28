#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>


/* The size of the array to be processed */
unsigned int array_size = 0;
unsigned int num_ts = 0;
/**
 * Merge two arrays already sorted
 * @param input_data is the array containing the two arrays to be merged
 * @param starting_cell is the index of the first cell of the first array
 * @param size is the sum of the sizes of the two arrays
 * @param output_data is where result has to be stored
 */
void bottom_up_merge(float * input_data, int starting_cell, int size, float * output_data) {
   if(starting_cell > array_size)
      return;

   /*The last position to be written */
   //if out of the bound of the array take just the array size (arrived to root)
   const int last_cell = (starting_cell + size <= array_size) ? starting_cell + size : array_size;

   /*The position in the left part of input data */
   int left_index = starting_cell;

   /*The position in the right part of input data */
   int right_index = starting_cell + size/2;

   /*The last position in the left part to be read*/
   const int last_left = (right_index < last_cell) ? right_index : last_cell;

    int index = starting_cell;

  //#pragma omp parallel for if(size > 5)
   for(index = starting_cell; index < last_cell; index++){
      if(left_index < last_left && (right_index >= last_cell || input_data[left_index] <= input_data[right_index])) {
         output_data[index] = input_data[left_index];
         left_index++;
      }
      else{
         output_data[index] = input_data[right_index];
         right_index++;
      }  
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

  float even_data[array_size];
  float odd_data[array_size];

  /* Initialize data in a random way */
  for(int index = 0; index < array_size; index++){
    unsigned int seed = index;
    odd_data[index] = rand_r(&seed) / (double) RAND_MAX;
  }
  int iteration = 0;

  float s = omp_get_wtime();


  for(unsigned int width = 2; width < 2 * array_size; width *= 2, iteration++){
    unsigned int sequence_number = array_size/width + (array_size % width != 0);
    if(iteration % 2 == 0){
      #pragma omp parallel for num_threads(num_ts)
      for(unsigned int j = 0; j < sequence_number; j++) 
        bottom_up_merge(odd_data, j * width, width, even_data);
    }else{
      #pragma omp parallel for num_threads(num_ts)
      for(unsigned int j = 0; j < sequence_number; j++) 
          bottom_up_merge(even_data, j * width, width, odd_data);  
    }
  }
  

  
  float e = omp_get_wtime();
  printf("Time %f\n", e-s);
  const float * final_data = iteration % 2 == 0 ? odd_data : even_data;

  isSorted(final_data) ? printf("sorted\n") : printf("not sorted\n");
  return 0;
}