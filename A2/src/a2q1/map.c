/* map.c
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic
 *  synchronization.
 *
 *  YOU MAY ADD WHATEVER YOU LIKE TO THIS FILE.
 *  YOU CANNOT CHANGE THE SIGNATURE OF MultithreadedWordCount.
 * ----------------------------------------------------------
 */
#include "data.h"

#include <stdlib.h>
#include <string.h>

struct countArgs
{
  struct Library * lib;
  int start;
  int end;
  char * word;
};


/* --------------------------------------------------------------------
 * countWords
 * --------------------------------------------------------------------
 * Takes a Library of articles and the index of the articls to count.
 * Returns the number of times that the word appears in the
 * given articles of the Library.
 * --------------------------------------------------------------------
 */
void * countWords(void * args){
  struct countArgs * arg = (struct countArgs *)args;
  struct Library * lib = arg->lib;
  int start = arg->start;
  int end = arg->end;
  char * word = arg->word;

  int * localCount = (int *)malloc(sizeof(int));
  *localCount = 0;
  for(int i = start; i <= end; i++) {
    struct Article * art = lib->articles[i];
    for(int j = 0; j < art->numWords; j++) {
      // Get the length of the function.
	    size_t len = strnlen( art->words[j], MAXWORDSIZE );
	    if ( !strncmp( art->words[j], word, len ) )
      {
        *localCount += 1;
      }
    }
  }
  
  //free(arg);
  return (void *)localCount;
}



/* --------------------------------------------------------------------
 * MultithreadedWordCount
 * --------------------------------------------------------------------
 * Takes a Library of articles containing words and a word.
 * Returns the total number of times that the word appears in the
 * Library.
 *
 * For example, "There the thesis sits on the theatre bench.", contains
 * 2 occurences of the word "the".
 * --------------------------------------------------------------------
 */

size_t MultithreadedWordCount( struct  Library * lib, char * word)
{
  printf("Parallelizing with %d threads...\n",NUMTHREADS);
    /* XXX FILLMEIN
     * Also feel free to remove the printf statement
     * to improve time */
    pthread_t threads[NUMTHREADS];
    void * retval;
    int avgArticles = lib->numArticles / NUMTHREADS;
    int remainder = lib->numArticles % NUMTHREADS;
    
    // Create threads to count words
    for(int i = 0; i < NUMTHREADS; i++) {
        struct countArgs * args = (struct countArgs *)malloc(sizeof(struct countArgs));
        args->lib = lib;
        if(i < remainder) {
            args->start = i * (avgArticles + 1);
            args->end = args->start + avgArticles;
        } else {
            args->start = i * avgArticles + remainder;
            args->end = args->start + avgArticles - 1;
        }
        args->word = word;
        pthread_create(&threads[i], NULL, countWords, (void *)args);
    }

    size_t count = 0;
    // Wait for threads to finish
    for(int i = 0; i < NUMTHREADS; i++) {
        pthread_join(threads[i], &retval);
        count += *(int *)retval;
    }

    // Return count
    return count;
}
