/**
 * @file sort.c
 * @brief This program is used to sort the numbers in a file.(fopen version)
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
		while (arr[j] >= curr)
			j--;
		while (arr[i] <= curr)
			i++;
		if (i < j)
			swap(&arr[i], &arr[j]);
	}
 
	swap(&arr[left],&arr[j]);
 
	QuickSort(arr,left,j - 1);
	QuickSort(arr,j + 1,right);
}

int main(int argc, char *argv[])
{
    //Record the start time by gettimeofday
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // n: number of numbers generated in the first line of the file
    FILE *fp = fopen("log.txt", "r");
    if (fp == NULL){
        printf("open file failed");
        exit(1);
    }
    
    int n;
    int rc = fscanf(fp, "%d", &n);
    if (rc == -1){
        printf("read file failed");
        exit(1);
    }
    printf("n = %d\n", n);

    int *nums = (int *)malloc(n * sizeof(int));

    // Read the numbers from the file
    for (int i = 0; i < n; i++)
    {
        // Read a line
        rc = fscanf(fp, "%d", &nums[i]);
        if (rc == -1){
            printf("read file failed");
            exit(1);
        }
        printf("num = %d\n", nums[i]);
    }

    fclose(fp);

    // Sort the numbers
    QuickSort(nums, 0, n - 1);

    //Print the sorted numbers
    for (int i = 0; i < n; i++)
    {
        printf("%d\t", nums[i]);
    }
    printf("\n");

    //Write the sorted numbers to a file
    FILE *fd = fopen("sorted.txt", "w");
    if (fd == NULL){
        printf("open file failed");
        exit(1);
    }
    
    
    rc = fprintf(fd, "%d\n", n);
    if (rc == -1){
        printf("write file failed");
        exit(1);
    }
    for (int i = 0; i < n; i++)
    {
        rc = fprintf(fd, "%d\n", nums[i]);
        if (rc == -1){
            printf("write file failed");
            exit(1);
        }
    }

    fclose(fd);
    free(nums);
    
    //Record the end time by gettimeofday
    gettimeofday(&end, NULL);
    printf("Operation took %ld.%08ld seconds\n", end.tv_sec - start.tv_sec, end.tv_usec - start.tv_usec);
     
    exit(0);
}
    


    
    