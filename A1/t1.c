#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>


int main(){
    int * a = 99;
    printf("%d\n", a);
    //free the pointer
    //free(a);
    if (a == NULL){
        printf("a is null");
    }
    int b = 0;
    printf("Is b false? %d\n", b);

    int *c = NULL;
    //int d = *c;
    //printf("d is %d\n", d);
    //if (d == NULL){
    //    printf("d is null");
    //}
    return 0;
}