#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>

#define RAND_COEFF 3.135164
#define MAX_FLOAT 10

int main(int argc, char **argv){
    
    float *array = mmap(
        NULL,
        sizeof(float)*MAX_FLOAT,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );

    if(array == MAP_FAILED){
        printf("Error in creating shared memory!");
        exit(1);
    }

    int f_c[2], c_f[2];
    pipe(f_c);
    pipe(c_f);

    int pid = fork();
    int token, trigger;
    srand(time(NULL) ^ (getpid()<<16));

    if(pid != 0){
        //I am in the father process now
        close(f_c[0]);
        close(c_f[1]);
        while(1){
            token = rand() % 10;
            for(int i = 0; i<token; i++){
                array[i] = (float) rand() * RAND_COEFF;
            }
            write(f_c[1], &token, sizeof(int));
            read(c_f[0], &trigger, sizeof(int));

        }
    }
    else{
        close(f_c[1]);
        close(c_f[0]);
        while(1){
            read(f_c[0], &token, sizeof(int));
            for(int i = 0; i<token; i++){
                printf("%.3f ", array[i]);
                printf("\n");
            }
            trigger = 1;
            write(c_f[1], &trigger, sizeof(int));

        }

    }



}

