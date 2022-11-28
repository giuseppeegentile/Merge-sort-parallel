#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>

//return the index of x in an array if present
//if not present return the position where it would have been hipotetically
int binary_search_lower(float* arr, int n, float value){
    // first greater or equal than target
    int start = 0, end = n - 1;
    while (start <= end)
    {
        int mid = (start + end) / 2;
        if (arr[mid] >= value)
            end = mid - 1;
        else
            start = mid + 1;
    }
    return start;
}

int binary_search_upper(float *arr,int n,float value){
    // first greater
    int start = 0, end = n - 1;
    while (start <= end)
    {
        int mid = (start + end) / 2;
        if (arr[mid] > value)
            end = mid - 1;
        else
            start = mid + 1;
    }
    return start;
}
/*** The size of the array to be processed */
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

   //The last position to be written 
   //if out of the bound of the array take just the array size (arrived to root)
   const int last_cell = (starting_cell + size <= array_size) ? starting_cell + size : array_size;

   //The position in the left part of input data 
   int left_index = starting_cell;

   //The position in the right part of input data 
   int right_index = starting_cell + size/2;

   //The last position in the left part to be read
   const int last_left = (right_index < last_cell) ? right_index : last_cell;

   int index = starting_cell;


    for(index = starting_cell; left_index < last_left && right_index < last_cell; index++){
        if(input_data[left_index] <= input_data[right_index]){
            output_data[index] = input_data[left_index];
            left_index++;
        }else{
            output_data[index] = input_data[right_index];
            right_index++;
        }   
    }


    //fastest way to copy arrays: memcpy 
    if(left_index < last_left){
        memcpy(&output_data[index], &input_data[left_index], (last_left - left_index) * sizeof(float));
        return; //if left-copy the right is already completed, avoiding another useless if
    }
  
    memcpy(&output_data[index], &input_data[right_index], (last_cell - right_index) * sizeof(float)); 
}

void bottom_up_merge_modified (float* input_data, int starting_cell, int size, float * output_data) {
    if(starting_cell > array_size)
      return;
    
    const int last_cell = (starting_cell + size <= array_size) ? starting_cell + size : array_size;
    int left_index = starting_cell;
    int right_index = starting_cell + size/2;
    const int last_left = (right_index < last_cell) ? right_index : last_cell;
    
    float right_part[last_cell - right_index];
    float left_part[last_left - starting_cell];
    float merged[last_cell - starting_cell];

    memcpy(&left_part[0], &input_data[starting_cell], sizeof(float) * (last_left - starting_cell));
    memcpy(&right_part[0], &input_data[right_index], sizeof(float) * (last_left - starting_cell));
    #pragma omp  parallel for num_threads(num_ts) shared(right_part, merged, left_part)
    for(int i = 0; i < (last_cell - right_index); i++){
        int pos = binary_search_lower(&left_part[0], (last_left - starting_cell), right_part[i]);
        merged[i + pos] = right_part[i];
        int pos2 = binary_search_upper(&right_part[0], (last_cell - right_index), left_part[i]);
        merged[i + pos2] = left_part[i];
    } 
    //memcpy(&output_data[starting_cell], &merged[0], sizeof(float) * (last_cell - starting_cell));
    for(int i = 0; i< last_cell - starting_cell; i++){
      output_data[starting_cell] = merged[i];
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
  float odd_data[array_size];

  /* Initialize data in a random way */
  for(int index = 0; index < array_size; index++){
    unsigned int seed = index;
    odd_data[index] = rand_r(&seed) / (double) RAND_MAX;
  }
  printf("\n");

  int iteration = 0;

  float s = omp_get_wtime();

  for(unsigned int width = 2; width < 2 * array_size; width *= 2, iteration++){
    unsigned int sequence_number = array_size/width + (array_size % width != 0);
      #pragma omp parallel for num_threads(num_ts)  if(width != 4)
      for(unsigned int  j = 0; j < sequence_number; j++) {
        if(width  == 4){
          bottom_up_merge_modified(odd_data, j * width, width, odd_data);
        }
        else
          bottom_up_merge(odd_data, j * width, width, odd_data);
      }
        
    if(width >= array_size /2 && num_ts > 2) //the bigger the chuncks, the less threads needed
      num_ts /= 2;
  }
  float e = omp_get_wtime();
  printf("Time %f\n", e-s);

  isSorted(odd_data) ? printf("sorted\n") : printf("not sorted\n");
  return 0;
}
