#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>

struct test
{
    int a;
    int b;
};

int main(){
    struct test *t1 = malloc(sizeof(struct test));
    t1->a = 1;
    t1->b = 2;
    printf("t1->a is %d\n", t1->a);
    printf("t1->b is %d\n", t1->b);
    struct test t2 = *t1;
    t2.a = 3;
    printf("t2->a is %d\n", t2.a);
    printf("t2->b is %d\n", t2.b);
    printf("t1->a is %d\n", t1->a);
    printf("t1->b is %d\n", t1->b);
    struct test *t3 = t1;
    free(t1);
    printf("t3->a is %d\n", t3->a);
    printf("t3->b is %d\n", t3->b);
    
    return 0;
}