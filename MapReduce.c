#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main (int argc, char *argv[]) {
   int i;
   double elapsed;
   elapsed = omp_get_wtime( );
   elapsed = omp_get_wtime() - elapsed;
}