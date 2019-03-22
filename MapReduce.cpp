#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <string.h>
#include <vector>
#include <iostream>

#include "concurrentqueue-master/concurrentqueue.h"

void readfile(){

}

std::vector <string> loadfiles() {

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

	// Declaring Vector of String type to store the file name
	std::vector <string> FileName;

	// Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    int len;
    char * last_four; 
    while ((de = readdir(dr)) != NULL){
    	// printf("%s\n", de->d_name); 
    	
    	// only use file with ".txt" extension
    	len = strlen(de->d_name);
    	if (len > 4){ //length of file name should be larger than 4 (".txt")
	    	last_four = &de->d_name[len-4];
	    	if ( (!strcmp(last_four, ".txt")) || (!strcmp(last_four, ".TXT")) ){
	    		printf("%s\n", de->d_name);
				std::FileName.push_back(de->d_name);
	    		// push this file to work queue
	    	}
	    }
    }
  
    closedir(dr);     
	return FileName;
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

int main (int argc, char *argv[]) {
   int i;
   double elapsed;
   elapsed = omp_get_wtime();

   // master thread initialize a concurrent work queue
   // refer to https://github.com/cameron314/concurrentqueue
   #pragma omp parallel
   {
   }

   // load all files from target directory
   loadfiles();

   elapsed = omp_get_wtime() - elapsed;
}