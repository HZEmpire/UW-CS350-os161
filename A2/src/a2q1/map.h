/* map.h
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic 
 *  synchronization.
 * ----------------------------------------------------------
 */

#ifndef ___MAP___
#define ___MAP___

#include "data.h"

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
size_t MultithreadedWordCount( struct  Library * lib, char * word );

#endif

