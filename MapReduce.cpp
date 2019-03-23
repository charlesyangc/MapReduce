#include <omp.h>
// #include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <string.h>

#include "concurrentqueue-master/concurrentqueue.h"

moodycamel::ConcurrentQueue<char *> input_file_queue;
moodycamel::ConcurrentQueue<char *> inter_file_queue[16];


void readfile(){

}

void loadfiles(){
	// refer to https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
	printf("===================================================================  loadfiles \n");
	struct dirent *de;  // Pointer for directory entry 
  
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir("./RawText/"); 

    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return; 
    } 

	// Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    int len;
    char * last_four; 
    int num_files = 0;
    while ((de = readdir(dr)) != NULL){
    	// printf("%s\n", de->d_name); 
    	
    	// only use file with ".txt" extension
    	len = strlen(de->d_name);
    	if (len > 4){ //length of file name should be larger than 4 (".txt")
	    	last_four = &de->d_name[len-4];
	    	if ( (!strcmp(last_four, ".txt")) || (!strcmp(last_four, ".TXT")) ){
	    		// printf("%s\n", de->d_name);
	    		// push this file to work queue
          input_file_queue.enqueue(de->d_name);
          num_files++;
	    	}
	    }
    }
    printf("in loadfiles, num_files = %d \n", num_files);
  
    closedir(dr);     
}

int java_hashCode(const char *str) {
	// refer to https://stackoverflow.com/questions/40303333/how-to-replicate-java-hashcode-in-c-language
	int i = 0;
    int hash = 0;

    for (i = 0; i < strlen(str); i++) {
        hash = 31 * hash + str[i];
    }

    return hash;
}

void map_function(){

}

void reduce_function(){

}

int main (int argc, char *argv[]) {
   int i;
   double elapsed;   

   // master thread initialize a concurrent work queue
   // refer to https://github.com/cameron314/concurrentqueue
   // moodycamel::ConcurrentQueue<char *> q;
   // test queue:
   // q.enqueue(25);
    // q.enqueue(40);
    // int test;
    // q.try_dequeue(test);
    // printf("test = %d \n", test);
    // q.try_dequeue(test);
    // printf("test = %d \n", test);
    // bool found = q.try_dequeue(test);
    // printf("found = %d \n", found);


   // open MP version
    elapsed = omp_get_wtime();
    // lead files and map
    int flag_loading_file_finished = 0;
    int num_files = 0;
    #pragma omp parallel
    {
      printf("get number of thread %d, thread rank: %d\n", omp_get_num_threads(), omp_get_thread_num());
      #pragma omp master
      {
        printf("master thread is %d \n", omp_get_thread_num());
        // master thread load file
        loadfiles();
        flag_loading_file_finished = 1;
      }
      // other thread read file
      while(flag_loading_file_finished == 0){
        char * file_name;
        bool found = input_file_queue.try_dequeue(file_name);
        if (found == 1){
          num_files++;
          printf("if found = %d \n", found);
          for (i = 0; i < strlen(file_name); i++){
            printf("%c",file_name[i]);
          }
          printf("\n");
        }
      }
    }
    printf("num_files = %d \n", num_files);
    // reduce
   elapsed = omp_get_wtime() - elapsed;


   // MPI version
   // elapsed = MPI_Wtime();
   //   int pid;
   //  int numP;

   //  MPI_Init(&argc, &argv);                 /* start MPI           */
   //  MPI_Comm_size(MPI_COMM_WORLD, &numP);   /* get number of ranks */
   //  MPI_Comm_rank(MPI_COMM_WORLD, &pid);   /* get rank            */
   //  if (pid == 0){
   //  printf("pid == 0 \n");    
   //  // load all files from target directory and enqueue these files
   //  loadfiles();
   //  // broadcast that loadfile has finished
   // }
   // if (pid != 0){ // all threads with pid != 0 is map/reduce thread
   //  // get work to read
   //  // input_file_queue.try_dequeue();
   // }
   // elapsed = MPI_Wtime() - elapsed;

   // pthreads version   
}