/* main.c
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic
 *  synchronization.
 * ----------------------------------------------------------
 */

// Library Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

// Local Includes
#include "data.h"
#include "map.h"

// Defines
#define MINSPEEDUP (2.0)
#define WORD ("abc")
#define TIMEOUT (10)

// Globals
int NUMARTICLES = 100;
int MINARTICLESIZE = 750000;
int MAXARTICLESIZE = 1000000;
int SEED = 0;

// Shorthand for error printing.
#define eprintf(...) fprintf(stderr, __VA_ARGS__)


/* --------------------------------------------------------------------
 * GenerateWord
 * --------------------------------------------------------------------
 * Produces a string of randomly generated letters over [a-z] with
 * a maximum length of MAXWORDSIZE.
 * --------------------------------------------------------------------
 */
char * GenerateWord()
{
    // Create space for word. The + 1 is for the \0 at the end of the string.
    unsigned int length = MINWORDSIZE + random() % (MAXWORDSIZE-MINWORDSIZE);
    char * word = ( char * )malloc( ( length + 1 ) * sizeof( char ) );
    if ( word == NULL ) {
	eprintf("Machine out of memory\n");
	exit(0);
    }

    // Fill word with random letters
    for ( unsigned int i = 0; i < length; i ++ )
    {
        unsigned int c = 'a' + (random() % ('z' - 'a'));
        word[i] = (char)c;
    }

    // All strings end with \0.
    word[length] = '\0';

    return word;
}

struct Article * GenerateArticle()
{
    // Create article
    struct Article * article = (struct Article *)malloc( sizeof( struct Article ) );

    // How many words?
    unsigned int numWords = MINARTICLESIZE + rand() % (MAXARTICLESIZE - MINARTICLESIZE);

    // Allocate space for words
    char ** words = (char **)malloc( numWords * sizeof( char * ));
    if ( words == 0 ) {
	eprintf("Machine out of memory\n");
	exit(0);
    }

    // Create words, add to article
    for ( unsigned int i = 0; i < numWords; i ++ )
    {
        words[i] = GenerateWord();
    }
    article->words = words;
    article->numWords = numWords;
    return article;
}

struct Library * GenerateLibrary()
{
    // Choose the number of articles
    unsigned int numArticles = NUMARTICLES;

    // Create Library
    struct Library * library = ( struct Library * )malloc( sizeof( struct Library ) );

    // Allocate space for articles
    library->articles = ( struct Article ** )malloc( numArticles * sizeof( struct Article * ) );

    // Produce articles
    for ( unsigned int i = 0; i < numArticles; i ++ )
    {
        library->articles[i] = GenerateArticle();
    }

    library->numArticles = numArticles;
    return library;
}

void FreeLibrary( struct Library * lib )
{
    for ( unsigned int i = 0; i < lib->numArticles; i ++ )
    {
        struct Article * art = lib->articles[i];
        for ( unsigned int j = 0; j < art->numWords; j ++)
        {
            char * word = art->words[j];
            free( word );
        }
        free( art->words );
        free( art );
    }

    free( lib->articles );
    free( lib );
}

// Single threaded version of the wordCount function.
size_t SingleThreadedWordCount( struct Library * lib, char * word)
{
    size_t wordCount = 0;

    for (unsigned int i = 0; i < lib->numArticles; i ++)
    {
	struct Article * art = lib->articles[i];
	for ( unsigned int j = 0; j < art->numWords; j++)
	{
	    // Get the length of the function.
	    size_t len = strnlen( art->words[j], MAXWORDSIZE );
	    if ( !strncmp( art->words[j], word, len ) )
	    {
		wordCount += 1;
	    }
	}
    }

    return wordCount;
}

// Type of the function used to count occurences.
typedef size_t (*counter)(struct Library *, char * word);

int CountOccurences( struct Library * lib, char * word, counter countFunc , double * time)
{
    struct timeval start, stop;
    gettimeofday(&start, NULL);
    size_t count = countFunc( lib, word);
    gettimeofday(&stop, NULL);

    // Pass the time elapsed to the caller.
    *time = (double)(stop.tv_sec - start.tv_sec) + (double)(stop.tv_usec - start.tv_usec) / (double)(1000 * 1000);

    return count;
}


int main( int argc, char ** argv )
{
  NUMTHREADS = -1;
    // Check for the inputs, parse them to ensure their validity.
    if ( argc != 4 )
    {
        printf( "Usage: a2q1 [NUMARTICLES] [SEED] [NUMTHREADS]\n" );
        return 0;
    }

    NUMARTICLES = strtol( argv[1], NULL, 10 );
    if ( NUMARTICLES == 0 ) {
	eprintf( "Invalid number of articles %s\n", argv[1] );
	return 0;
    }

    SEED = strtol( argv[2], NULL , 10 );
    if ( SEED == 0 ) {
	eprintf( "Invalid seed %s\n", argv[2] );
  return 0;
}

    NUMTHREADS = strtol(argv[3], NULL, 10);
    if(NUMTHREADS <= 0){
      eprintf( "Invalid NUMTHREADS %s\n", argv[3] );
	return 0;
    }

    // Get an arbitrary valid string to search for
    char * word = WORD;

    // Init the random number generator, create the libraries.
    srandom( SEED );

    // Allocate space for the library of articles
    struct Library * lib = GenerateLibrary();

    // Measuring time
    double timeSingle, timeMulti;

    alarm(TIMEOUT);

    int countSingle = CountOccurences( lib, word, SingleThreadedWordCount, &timeSingle);
    int countMulti = CountOccurences( lib, word, MultithreadedWordCount, &timeMulti);

    if ( countSingle != countMulti )
    {
	eprintf( "ERROR: Single threaded version found %d occurences.\n", countSingle );
	eprintf( "ERROR: Multi threaded version found %d occurences.\n", countMulti );
	eprintf( "ERROR: Please check for race conditions or other bugs.\n" );
	exit(1);
    }

    double speedupRatio = timeSingle / timeMulti;
    if ( speedupRatio < MINSPEEDUP )
    {
	eprintf( "ERROR: Speedup is %f, less than %f\n", speedupRatio, MINSPEEDUP );
	eprintf( "ERROR: Single Threaded is %fs, mulithreaded is %fs\n", timeSingle, timeMulti);
	eprintf( "ERROR: Please fix any bottlenecks in the code.\n" );
	exit(1);
    }

    printf( "Found %d occurences of %s.\n", countSingle, word );

    FreeLibrary( lib );

    return 0;
}
