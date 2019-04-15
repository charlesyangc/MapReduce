#include <omp.h>
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

#define NUM_THREADS 7

struct wordcounttuple
{
  std::string word;
  int count; 
};

int flag_reading_file_finished = 0;

moodycamel::ConcurrentQueue<std::string> input_file_queue;
moodycamel::ConcurrentQueue<wordcounttuple> inter_file_queue[NUM_THREADS];

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
int HashMap(std::string word, int num_reducer) {

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


void readfile(std::string fileName){
	// Store word count in map type
	std::map<std::string, int> WordCount;

	// Open file
	std::ifstream file;
	file.open("./RawText/" + fileName);
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

	}

    // ver.2
	std::map<std::string, int>::iterator itr;
	wordcounttuple workcount;
	int reducer_id;
	for (itr = WordCount.begin(); itr != WordCount.end(); ++itr) {
	reducer_id = HashMap(itr->first, NUM_THREADS);
	workcount.word = itr->first;
	workcount.count = itr->second;
	inter_file_queue[reducer_id].enqueue(workcount);
	}  

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

void reduce_function(int reducer_id){
	printf("in reduce_function, reducer_id = %d \n", reducer_id);
	std::string newWord, Count_newWord;

	// Store word count in map type
	std::map<std::string, int> WordCount;

	wordcounttuple wordcount;
	while (flag_reading_file_finished == 0) {
		if (inter_file_queue[reducer_id].try_dequeue(wordcount) == 1) {
			WordCount[wordcount.word] += wordcount.count;
		}
	}
	while (inter_file_queue[reducer_id].try_dequeue(wordcount) == 1 ) {
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

void print_reading_status(int thread_id, std::string file_name) {
	std::cout << "Thread " << thread_id << " is reading file: " << file_name << std::endl;
}

int main (int argc, char *argv[]) {
   int i;
   double elapsed;   

   // master thread initialize a concurrent work queue
   // refer to https://github.com/cameron314/concurrentqueue

   // define the number of mappers and reducers
   omp_set_nested(1);   /* make sure nested parallism is on */
   int num_reducers = 3; // NUM_THREADS / 3;
   int num_mappers = NUM_THREADS - num_reducers;

	// open MP version
	elapsed = omp_get_wtime();

	// Set number of threads
	omp_set_num_threads(NUM_THREADS);

	// lead files and map
	int flag_loading_file_finished = 0;
	int num_files = 0;
	#pragma omp parallel default(none) shared(num_reducers, num_mappers, flag_reading_file_finished, num_files, flag_loading_file_finished, input_file_queue) num_threads(2)
	{
		// master thread loads all file names, enqueues the names, and assigns two thread groups for mapper and reducer
		#pragma omp master 
		{
			printf("master thread is %d \n", omp_get_thread_num());
			loadfiles();
			flag_loading_file_finished = 1;

			// Enqueue a file name "#END_READING_FILE#" at the end of input_file_queue to indicate the last file name
			input_file_queue.enqueue("#END_READING_FILE#");

			// Refer to: https://stackoverflow.com/questions/25556748/openmp-divide-all-the-threads-into-different-groups
			// Readers and Mappers, nested parallel task, num_threads = num_mappers
			#pragma omp task
			#pragma omp parallel num_threads(num_mappers)
			{
				std::string file_name;
				while (input_file_queue.try_dequeue(file_name) == 1) {
					if (file_name == "#END_READING_FILE#")
						flag_reading_file_finished = 1;
					else {
						num_files++;
						print_reading_status(omp_get_thread_num(), file_name);
						readfile(file_name);
					}
				}
			}

			// Reducers, nested parallel task, num_threads = num_mappers
			#pragma omp task
			#pragma omp parallel num_threads(num_reducers)
			{
				reduce_function(omp_get_thread_num());
			}
		}
	}

	printf("number of files read is %d \n", num_files);
	elapsed = omp_get_wtime() - elapsed;
	printf("elapsed time is %.10f s\n", elapsed);
}