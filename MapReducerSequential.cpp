#include <omp.h>
// #include <mpi.h>
// #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <map>
#include <iterator>

#include "concurrentqueue-master/concurrentqueue.h"

struct wordcounttuple
{
  std::string word;
  int count; 
};

// moodycamel::ConcurrentQueue<char *> input_file_queue;
moodycamel::ConcurrentQueue<std::string> input_file_queue;
// moodycamel::ConcurrentQueue<std::string> inter_file_queue[NUM_THREADS];
moodycamel::ConcurrentQueue<wordcounttuple> inter_file_queue[1];

void printcharpointer(char * a){
  int i;
  for (i = 0; i < strlen(a); i++){
    printf("%c",a[i]);
  }
  printf("\n");
}

void sequential_readfile(std::string fileName){
  // Store word count in map type
  std::map<std::string, int> WordCount;

  // Open file
  std::ifstream file;
  file.open((std::string)"./RawText/" + fileName);
  if (!file.is_open()) {
    std::cout << "Fail to open the file: " << fileName << std::endl;
    return;
  }

  // Read word by word from the file
  std::string word;
  while (file >> word) {
    // remove punctuations in each word phrase
    word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c) { return std::ispunct(c);}), word.end());
    
    // Convert upper case to lower case
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);

    // Count the number of each word locally
    ++WordCount[word];

    // print each word
    // std::cout << word << std::endl;
  }

  std::map<std::string, int>::iterator itr;
  wordcounttuple workcount;
  for (itr = WordCount.begin(); itr != WordCount.end(); ++itr) {
    workcount.word = itr->first;
    workcount.count = itr->second;
    inter_file_queue[0].enqueue(workcount);
  }  

  // Close file
  file.close();
}

void sequential_loadfiles(){
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
          std::string stringname = (std::string)de->d_name;
          input_file_queue.enqueue(stringname);
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

void reduce_function(int reducer_id){
  printf("in reduce_function, reducer_id = %d \n", reducer_id);
	std::string newWord, Count_newWord;

	// Store word count in map type
	std::map<std::string, int> WordCount;

    // dequeue the file name
	// std::string file_name;
	// while (inter_file_queue[reducer_id].try_dequeue(file_name) == 1) {
	// 	// Open file
	// 	std::ifstream file;
	// 	file.open(file_name);
	// 	if (!file.is_open()) {
	// 		std::cout << "Fail to open the file: " << file_name << std::endl;
	// 		return;
	// 	}

	// 	// read new word and its count from file 
	// 	file >> newWord;
	// 	file >> Count_newWord;

	// 	// add up the counts
 //    // std::cout << "in reduce_function" << std::endl;
 //    // std::cout << newWord << std::endl;
 //    if (Count_newWord.length()>0){
 //      int counts = std::stoi(Count_newWord, nullptr);
 //      WordCount[newWord] += counts;
 //      // if (reducer_id == 0)
 //      //   std::cout << newWord << WordCount[newWord] << std::endl;
 //    }

	// 	// delete the temperary file
	// 	file.close();
 //    const char * c = file_name.c_str();
	// 	std::remove(c);
	// }

  wordcounttuple wordcount;
  while (inter_file_queue[reducer_id].try_dequeue(wordcount) == 1) {
    WordCount[wordcount.word] += wordcount.count;
  }



	// store the count in a file named after the reducer id
	std::map<std::string, int>::iterator itr;
	std::ofstream ofs;
	std::string Output_fileName = "Output_from_reducer" + std::to_string( reducer_id )+ ".txt";
	ofs.open(Output_fileName, std::ofstream::out | std::ofstream::trunc);
	for (itr = WordCount.begin(); itr != WordCount.end(); ++itr)
		ofs << itr->first << ' ' << itr->second << '\n';
	ofs.close();

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

    // sequential version
   elapsed = omp_get_wtime();

    sequential_loadfiles();
    std::string file_name;
    while (input_file_queue.try_dequeue(file_name) == 1){
      sequential_readfile(file_name);
    }
    reduce_function(0);
    elapsed = omp_get_wtime() - elapsed;
    printf("elapsed is %.10f \n", elapsed);

  

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