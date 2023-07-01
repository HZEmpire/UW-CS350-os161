/* data.h
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic
 *  synchronization.
 * ----------------------------------------------------------
 */

#ifndef ___DATA___
#define ___DATA___

#include <pthread.h>
#include <stdio.h>
#include <string.h>

struct Article
{
    char ** words;
    unsigned int numWords;
};

struct Library
{
    struct Article ** articles;
    unsigned int numArticles;
};

#define MAXWORDSIZE 4
#define MINWORDSIZE 3

int NUMTHREADS;

#endif
