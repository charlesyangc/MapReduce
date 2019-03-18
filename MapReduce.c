#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <string.h>

void readfile(){

}

void loadfiles(){
	printf("loadfiles \n");
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
    while ((de = readdir(dr)) != NULL) 
            printf("%s\n", de->d_name); 
  
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

int main (int argc, char *argv[]) {
   int i;
   double elapsed;
   elapsed = omp_get_wtime();

   // load all files from target directory
   loadfiles();

   elapsed = omp_get_wtime() - elapsed;
}