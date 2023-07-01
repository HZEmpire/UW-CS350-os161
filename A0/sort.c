/**
 * @file sort.c
 * @brief This program is used to sort the numbers in a file.
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

/**
 * Define a swap function
 * 
 */
void swap(int *a, int *b)
{
    int mid = *a;
    *a = *b;
    *b = mid;
    return;
}


/**
 * Define a Quick sort function
 * 
 */
void QuickSort(int *arr,int left,int right)
{
	if (left >= right)
		return;
	int i, j;
	int curr = arr[left];
	i = left, j = right;
	while (i < j)
	{
		while (arr[j] >= curr && i < j)
			j--;
		while (arr[i] <= curr && i < j)
			i++;
		if (i < j)
			swap(&arr[i], &arr[j]);
	}
 
	swap(&arr[left],&arr[j]);
 
	QuickSort(arr,left,j - 1);
	QuickSort(arr,j + 1,right);
}

/**
 * Define a function to read a line from the file
 * 
 */
void readLine(int fd, char *buf)
{
    char current[2];
    int rc;
    int i = 0;
    while(1)
    {
        rc = read(fd, current, 1);
        if (rc == -1){
            printf("read file failed");
            exit(1);
        }
        if (current[0] == '\n')
            break;
        buf[i] = current[0];
        i++;
    }
    buf[i] = '\0';
    return;
}

int main(int argc, char *argv[])
{
    //Record the start time by gettimeofday
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // n: number of numbers generated in the first line of the file
    int fd = open("log.txt", O_RDONLY);
    if (fd == -1){
        printf("open file failed");
        exit(1);
    }
    
    char buf[15];
    readLine(fd, buf);
    int n = atoi(buf);

    int *nums = (int *)malloc(n * sizeof(int));

    // Read the numbers from the file
    for (int i = 0; i < n; i++)
    {
        // Read a line
        readLine(fd, buf);
        nums[i] = atoi(buf);
    }

    close(fd);

    // Sort the numbers
    QuickSort(nums, 0, n - 1);

    //Write the sorted numbers to a file
    fd = open("sorted.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1){
        printf("open file failed");
        exit(1);
    }
    
    char num[15];
    sprintf(num, "%d\n", n);
    int rc = write(fd, num, strlen(num));
    if (rc == -1){
        printf("write file failed");
        exit(1);
    }
    for (int i = 0; i < n; i++)
    {
        sprintf(num, "%d\n", nums[i]);
        rc = write(fd, num, strlen(num));
        if (rc == -1){
            printf("write file failed");
            exit(1);
        }
    }

    close(fd);
    free(nums);

    //Record the end time by gettimeofday
    gettimeofday(&end, NULL);
    printf("Operation took %ld.%08ld seconds\n", end.tv_sec - start.tv_sec, end.tv_usec - start.tv_usec);
    
    exit(0);
}
    


    
    