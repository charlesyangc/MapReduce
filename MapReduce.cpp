#include <omp.h>
// #include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <iterator>

#include "concurrentqueue-master/concurrentqueue.h"

moodycamel::ConcurrentQueue<char *> input_file_queue;
moodycamel::ConcurrentQueue<std::string> inter_file_queue[3];

void printcharpointer(char * a){
  int i;
  for (i = 0; i < strlen(a); i++){
    printf("%c",a[i]);
  }
  printf("\n");
}


// Relative frequencies of the first letters of a word in the English language
// source: https://en.wikipedia.org/wiki/Letter_frequency
//std::map<char, float> FreqFirstLetter = {
//        { 'a', 0.11682 }, { 'b', 0.04434 }, { 'c', 0.05238 }, { 'd', 0.03174 }, { 'e', 0.02799 },
//        { 'f', 0.04027 }, { 'g', 0.01642 }, { 'h', 0.04200 }, { 'i', 0.07294 }, { 'j', 0.00511 },
//        { 'k', 0.00456 }, { 'l', 0.02415 }, { 'm', 0.03826 }, { 'n', 0.02284 }, { 'o', 0.07631 },
//        { 'p', 0.04319 }, { 'q', 0.00222 }, { 'r', 0.02826 }, { 's', 0.06686 }, { 't', 0.15978 },
//        { 'u', 0.01183 }, { 'v', 0.00824 }, { 'w', 0.05497 }, { 'x', 0.00045 }, { 'y', 0.00763 },
//        { 'z', 0.00045 } };
std::map<char, float> FreqFirstLetter = {
        { 'a', 0.11682 }, { 'b', 0.1612 }, { 'c', 0.2135 }, { 'd', 0.2453 }, { 'e', 0.2733 },
        { 'f', 0.3135 }, { 'g', 0.3300 }, { 'h', 0.3720 }, { 'i', 0.4449 }, { 'j', 0.4500 },
        { 'k', 0.4546 }, { 'l', 0.4787 }, { 'm', 0.5170 }, { 'n', 0.5398 }, { 'o', 0.6161 },
        { 'p', 0.6593 }, { 'q', 0.6615 }, { 'r', 0.6898 }, { 's', 0.7567 }, { 't', 0.9164 },
        { 'u', 0.9283 }, { 'v', 0.9365 }, { 'w', 0.9915 }, { 'x', 0.9919 }, { 'y', 0.9996 },
        { 'z', 1.0000 } };

// Map word to reducer based on its first letter, return the index of reducer 0 : num_reducer-1
int HashMap(std::string& word, int num_reducer) {

  int AssignedReducer = 0;

  try {
    char FirstLetter = word.at(0);
    AssignedReducer = (int)(FreqFirstLetter.at(FirstLetter) * num_reducer);
  }
  catch (...) { // assign other cases to the last reducer
    AssignedReducer = num_reducer - 1;
  }

  if (AssignedReducer == num_reducer)
    return (AssignedReducer - 1);
  else
    return AssignedReducer;
}


void readfile(char * fileName){
	// Store word count in map type
	std::map<std::string, int> WordCount;

	// Open file
	std::ifstream file;
	file.open((std::string)"./RawText/" + (std::string)fileName);
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

  // output intermediate scratch file
	std::map<std::string, int>::iterator itr;
  std::ofstream ofs;
  std::string inter_file_name;
  int reducer_id;
  for (itr = WordCount.begin(); itr != WordCount.end(); ++itr) {
    inter_file_name = fileName + itr->first;
    ofs.open (inter_file_name, std::ofstream::out | std::ofstream::trunc);
    ofs << itr->first << '\t' << itr->second << '\n';
    ofs.close();
    reducer_id = HashMap(inter_file_name, 3);
    inter_file_queue[reducer_id].enqueue(inter_file_name);
  }
  // Print the word count from map type
	// std::cout << "\nThe map WordCount is : \n";
	// std::cout << "\tWord\tCount\n";
	// for (itr = WordCount.begin(); itr != WordCount.end(); ++itr) {
	// 	std::cout << '\t' << itr->first
	// 		<< '\t' << itr->second << '\n';
	// }

	// Close file
	file.close();
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

void reduce_function(int reducer_id){
	std::string newWord, Count_newWord;

	// Store word count in map type
	std::map<std::string, int> WordCount;

    // dequeue the file name
	std::string file_name;
	while (inter_file_queue[reducer_id].try_dequeue(file_name) == 1) {
		// Open file
		std::ifstream file;
		file.open(file_name);
		if (!file.is_open()) {
			std::cout << "Fail to open the file: " << fileName << std::endl;
			return;
		}

		// read new word and its count from file 
		file >> newWord;
		file >> Count_newWord;

		// add up the counts
		WordCount[word] += std::stoi(Count_newWord);
	}

	// store the count in a file named after the 
	std::map<std::string, int>::iterator itr;
	std::ofstream ofs;
	std::string Output_fileName = "Output from reducer" + (std::string) reducer_id + ".txt";
	ofs.open(Output_fileName, std::ofstream::out | std::ofstream::trunc);
	for (itr = WordCount.begin(); itr != WordCount.end(); ++itr)
		ofs << itr->first << '\t' << itr->second << '\n';
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


  // open MP version
  elapsed = omp_get_wtime();

	// 2 threads for testing
	omp_set_num_threads(3);

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
    if (omp_get_thread_num() != 0){
      char * file_name;
      bool found;
      printf("read thread rank: %d\n", omp_get_thread_num());
      do{
        found = input_file_queue.try_dequeue(file_name);
        if (found == 1){
          num_files++;
          printf("if found = %d, thread is %d \n", found, omp_get_thread_num());
          printcharpointer(file_name);
          readfile(file_name);
        }
      }while(flag_loading_file_finished == 0);
      while (input_file_queue.try_dequeue(file_name) == 1){
        num_files++;
        printf("if found = %d, thread is %d \n", found, omp_get_thread_num());
        printcharpointer(file_name);
        readfile(file_name);
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