/**
 * @file make_numbers.c
 * @brief This program is used to generate a list of random numbers and write them to a file.
 * @author Xu Haozhou
 * @version 1.0
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
    //Record the start time by gettimeofday
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // n: number of numbers to generate
    // hi: upper bound of the random numbers
    // lo: lower bound of the random numbers
    int n, hi, lo;

    if (argc != 4)
        exit(1);
    
    n = atoi(argv[1]);
    lo = atoi(argv[2]);
    hi = atoi(argv[3]);
    
    if (n <= 0 || lo > hi)
        exit(1);

    int fd = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
        exit(1);

    //First write n to the file
    char buf[15];
    sprintf(buf, "%d\n", n);
    int rc = write(fd, buf, strlen(buf));
    if (rc == -1)
        exit(1);

    for (int i = 0; i < n; i++)
    {
        srand(i + time(NULL));
        int num = lo + rand() % (hi - lo + 1);
        sprintf(buf, "%d\n", num);
        rc = write(fd, buf, strlen(buf));
        if (rc == -1)
            exit(1);
    }

    close(fd);

    //Record the end time by gettimeofday
    gettimeofday(&end, NULL);
    printf("Operation took %ld.%08ld seconds\n", end.tv_sec - start.tv_sec, end.tv_usec - start.tv_usec);
    
    exit(0);
}