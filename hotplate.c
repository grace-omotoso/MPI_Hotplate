/* A program to solve the hotplate problem in a parallel fashion having a near 
 * linear speed up as thread increase
 
 Author: Bukola Grace Omotoso
 MNumber: M01424979
 ID: bgo2e
 Last Code Clean-up: 10/22/2018, 2:20am
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include "mpi.h"

/* Build memory structure for hotplate*/
float** buildHotplate(int rows, int columns) {
    float** hotplate;
    hotplate = (float**) malloc(rows*sizeof(float*));
    for (int i = 0; i < rows; i++)
        hotplate[i] = (float*) malloc(columns*sizeof(float));
    return hotplate;
}

void initializeHotPlate(int num_rows, int num_cols, float** hotplate, float** hotplateClone, int top_temp, int left_temp, int right_temp, int bottom_temp)    {
    int num_outer_grid = (2 * num_rows) + (2 * (num_cols - 2));
    float outer_grid_sum = (top_temp * (num_cols - 2)) + (left_temp * (num_rows - 1)) + (bottom_temp * num_cols) + (right_temp * (num_rows - 1));
    float initial_inner_val = outer_grid_sum / num_outer_grid;
    
    for (int row = 0; row < num_rows; row++) {
        for (int column = 0; column < num_cols; column++) {
            
            //top values override the top row except the edges
            if ((row == 0) & (column != 0 & column != num_cols - 1)) {
                hotplate[row][column] = top_temp;
                hotplateClone[row][column] = top_temp;
            }
            else if (column == 0 && (row != (num_rows-1))) {
                hotplate[row][column] = left_temp;
                hotplateClone[row][column] = left_temp;
            }
            else if (column == (num_cols - 1) && (row != (num_rows-1))) {
                hotplate[row][column] = right_temp;
                hotplateClone[row][column] = right_temp;
            }
            else if(row == (num_rows -1 )){
                hotplate[row][column] = bottom_temp;
                hotplateClone[row][column] = bottom_temp;
            }
            if ((row != 0) && (row != num_rows - 1) && (column != 0) && (column != num_cols - 1))
                hotplate[row][column] = initial_inner_val;
        }
    }
    
}

/*Get the maximum values from all threads*/
float max_max_diff(float arr[], int n)
{
    int i;
        float max = arr[0];
    for (i = 1; i < n; i++)
        if (arr[i] > max)
            max = arr[i];
    return max;
}

/* Swap hotplate and its clone*/

void swapHotplate(float *a, float *b) {
    
    float tmp = *a;
    *a = *b;
    *b = tmp;
}


void swapHotplate2(float **hotplate, float **hotplateClone, int num_rows, int num_cols, int offset, int stop, int proc_id) {
    if (offset == 1)
    offset--;
    if (stop >= num_rows)
		stop = num_rows - 1;
    float *tmp;

    for(int row  =  offset;  row < stop; row++){
		 		for(int col = 0; col < num_cols; col++){
			float tmp = hotplate[row][col];
			hotplate[row][col] = hotplateClone[row][col];
			hotplateClone[row][col] = tmp;
		}
	}
}

/* Get current time*/

double timestamp()
{
    struct timeval tval;
    
    gettimeofday( &tval, ( struct timezone * ) 0 );
    return ( tval.tv_sec + (tval.tv_usec / 1000000.0) );
}



float generateHeat2(float** hotplate, float** hotplateClone, int  offset, int chunksize, int proc_id, int num_rows, int num_cols){

	
	 int start_row = (proc_id - 1)*chunksize;
      int stop_row = proc_id*chunksize;
	 if (proc_id  == 1)
		start_row = 1;
    if (stop_row >= num_rows)
		stop_row = num_rows-1;
	
	float max_difference = 0;
    float previous_val;
    float current_val;
    float diff, max_epsilon_val;

	    for (int row = start_row; row < stop_row; row++) {
	
            for (int column = 1; column < (num_cols - 1); column++) {
            previous_val = hotplate[row][column];
            current_val = ((hotplate[row - 1][column] + hotplate[row][column - 1] + hotplate[row + 1][column] + hotplate[row][column + 1]) / 4);
            diff = fabsf(previous_val - current_val);
    
            if (diff > max_difference){
                max_difference = diff;
             
            }
            hotplateClone[row][column] = current_val;
            
        }
        
    }
            swapHotplate2(hotplate, hotplateClone, num_rows, num_cols, start_row, stop_row, proc_id);

    return max_difference;
	
}

int main(int argc, char *argv[])
{
	float max_epsilon_val;
	long int sum, partial_sum;
    int num_rows = atoi(argv[1]);
    int num_cols = atoi(argv[2]);
    int top_temp = atoi(argv[3]);
    int left_temp = atoi(argv[4]);
    int right_temp = atoi(argv[5]);
    int bottom_temp = atoi(argv[6]);
    float epsilon = atof(argv[7]);
    float** hotplate;
float** hotplateClone;
        
    hotplate =  buildHotplate(num_rows, num_cols);
    hotplateClone = buildHotplate(num_rows, num_cols);
    
    initializeHotPlate(num_rows, num_cols, hotplate, hotplateClone, top_temp, left_temp, right_temp, bottom_temp);
	
	
	float max_difference = 0;
	float global_max = epsilon + 1;
    float previous_val;
    float current_val;
    float diff, max_epsilon_val_received;
    double begin, end;
      MPI_Status status;
      int rc, proc_id, mgr_process = 0, i, num_procs,
         cur_id, start_row_to_receive, row_per_proc, 
         sender, start_row_received, start_row, stop_row, start_row_to_send, offset, chunksize, tag1, tag2, tag3, tag4;


       float cur_max_epsilon =  epsilon + 1;
        float cur_max_difference = 0.0;
       int counter = 0;

      

       MPI_Init(&argc, &argv);
       rc = MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
	   rc = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);	
       
       int working_proc = num_procs - 1;
       chunksize =  (num_rows + working_proc - 1)/working_proc;
              
       tag2 = 1;
       tag1 = 2;
       tag3 = 4;
       tag4 = 3;
       int break_from_loop = 0;
       
		begin = timestamp();
     while(break_from_loop != 1){
		
if(proc_id == mgr_process){
       alarm(180);
  /* Send each task its portion of the array - master keeps 1st part */
  offset = 0;
  for (int dest=1; dest<=working_proc; dest++) {
    MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
    MPI_Send(&hotplate[offset][0], chunksize, MPI_FLOAT, dest, tag2, MPI_COMM_WORLD);
    MPI_Send(&hotplateClone[offset][0], chunksize, MPI_FLOAT, dest, tag3, MPI_COMM_WORLD);

    offset = offset + chunksize;
    }

  for (i=1; i<=working_proc; i++) {
    int source = i;

   MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);

  MPI_Recv(&hotplate[offset][0], chunksize, MPI_FLOAT, source, tag2, MPI_COMM_WORLD, &status);

  MPI_Recv(&hotplateClone[offset][0], chunksize, MPI_FLOAT, source, tag3, MPI_COMM_WORLD, &status);

    }

  /* Get final sum and print sample results */  
  MPI_Reduce(&max_difference, &global_max, 1, MPI_FLOAT, MPI_MAX, mgr_process, MPI_COMM_WORLD);
  if ((counter > 0 && (counter & (counter - 1)) == 0 )|| global_max < epsilon)
    printf("%-10d%10.6f\n", counter, global_max);
    
    if (global_max < epsilon){
		break_from_loop = 1;
		end = timestamp();
		 printf("%s%5.2f\n","TOTAL TIME: ", (end-begin));
	}
    counter++;
  
}


if (proc_id > mgr_process) {
alarm(180);
  /* Receive my portion of array from the master task */
  int source = mgr_process;
  MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
  MPI_Recv(&hotplate[offset][0], chunksize, MPI_FLOAT, source, tag2, 
    MPI_COMM_WORLD, &status);
      MPI_Recv(&hotplateClone[offset][0], chunksize, MPI_FLOAT, source, tag3, 
    MPI_COMM_WORLD, &status);

  max_difference = generateHeat2(hotplate, hotplateClone, offset, chunksize, proc_id, num_rows, num_cols);


  MPI_Send(&offset, 1, MPI_INT, mgr_process, tag1, MPI_COMM_WORLD);
  MPI_Send(&hotplate[offset][0], chunksize, MPI_FLOAT, mgr_process, tag2, MPI_COMM_WORLD);
  MPI_Send(&hotplateClone[offset][0], chunksize, MPI_FLOAT, mgr_process, tag3, MPI_COMM_WORLD);
  MPI_Reduce(&max_difference, &global_max, 1, MPI_FLOAT, MPI_MAX, mgr_process, MPI_COMM_WORLD);
}
   MPI_Bcast(&break_from_loop, 1, MPI_INT, mgr_process, MPI_COMM_WORLD);
}

MPI_Finalize();


    
    return 0;
}
