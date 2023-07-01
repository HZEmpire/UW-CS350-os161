#include <pthread.h>

struct resource {
	int counter;			/* Number of operations */
	long num_consumers;		/* Number of active consumers in the resource */
	long num_producers;		/* Number of active producers in the resource */
	int ratio;			/* Ratio of producers to consumers */
	pthread_cond_t cond;		/* Resource condition variable */
	pthread_mutex_t mutex;		/* Resource mutex */
};


