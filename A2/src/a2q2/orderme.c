#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "assignment.h"

/*
 * Note: Error handling is deliberatly gratuitous; we are a short lived
 * userspace utility, and we exit on every error, so we will be properly cleaned
 * up anyway. Providing a good example of clean C is worth the extra effort,
 * though.
 */

pthread_mutex_t done_mutex= PTHREAD_MUTEX_INITIALIZER;
bool done = false;

bool
check_done(void)
{
	bool ret;

	pthread_mutex_lock(&done_mutex);
	ret = done;
	pthread_mutex_unlock(&done_mutex);

	return (ret);
}

void
set_done(bool val)
{
	pthread_mutex_lock(&done_mutex);
	done = val;
	pthread_mutex_unlock(&done_mutex);
}

void
assert_capacity(struct resource *resource)
{
	assert(resource->num_consumers <= resource->num_producers * resource->ratio);
}

/* Sleep for an arbitrary amount of time. */
void
compute(void)
{
	usleep(1000 * (time(NULL) % 10));
}

/* Sleep for an arbitrary amount of time. */
void
rest(void)
{
	usleep(1000 * (time(NULL) % 50));
}

/* Functions for the consumers and producers. */
void *
consume(void *data)
{
	struct resource *resource = (struct resource *) data;
	int *ret;

	ret = malloc(sizeof(*ret));
	if (ret == NULL) {
		perror("malloc");
		pthread_exit(NULL);
	}

	while (!check_done()) {
		consume_enter(resource);

		assert_capacity(resource);

		/* Computation happens outside the critical section.*/
		pthread_mutex_unlock(&resource->mutex);
		compute();

		pthread_mutex_lock(&resource->mutex);

		consume_exit(resource);

		/* Wait for a bit. */
		rest();
	}

	*ret = 0;
	pthread_exit(ret);
}

void *
produce(void *data)
{
	struct resource *resource = (struct resource *) data;
	int *ret;

	ret = malloc(sizeof(*ret));
	if (ret == NULL) {
		perror("malloc");
		pthread_exit(NULL);
	}

	while (!check_done()) {
		produce_enter(resource);

		assert_capacity(resource);

		/* Computation happens outside the critical section.*/
		pthread_mutex_unlock(&resource->mutex);
		compute();

		/*
		 * Edge condition to avoid deadlocking consumers.
		 * If we didn't have this and all producers exited
		 * while there were consumers waiting, the consumers
		 * would starve.
		 */
		if (check_done())
		    break;

		pthread_mutex_lock(&resource->mutex);

		produce_exit(resource);

		/* Wait for a bit. */
		rest();
	}


	*ret = 0;
	pthread_exit(ret);
}

struct resource *
resource_setup(long num_consumers, long num_producers, long ratio)
{
	struct resource *resource;
	int error;

	resource = calloc(1, sizeof(*resource));
	if (resource == NULL) {
		perror("calloc");
		return (NULL);
	}

	error = pthread_cond_init(&resource->cond, NULL);
	if (error != 0) {
		fprintf(stderr, "pthread_cond_init: %s", strerror(error));
		return (NULL);
	}

	error = pthread_mutex_init(&resource->mutex, NULL);
	if (error != 0) {
		fprintf(stderr, "pthread_mutex_init: %s", strerror(error));
		pthread_cond_destroy(&resource->cond);
		return (NULL);
	}

	/* Make sure the values are sane. */
	assert(num_consumers > 0);
	assert(num_producers > 0);
	assert(ratio > 0);
	assert(num_producers * ratio >= num_consumers);

	/* No active producers or consumers. */
	resource->num_consumers = 0;
	resource->num_producers = 0;
	resource->ratio = ratio;

	/* The upper limit of consumers starts at 0, there are no producers. */

	return (resource);
}

/*
 * This function gets called either before thread creation or after exit, so
 * there is no concurrency.
 */
void
resource_teardown(struct resource *resource)
{
	assert(resource->num_consumers == 0);

	pthread_cond_destroy(&resource->cond);
	pthread_mutex_destroy(&resource->mutex);

	free(resource);
}

void
thread_teardown(pthread_t *threads, struct resource *resource, int nthreads)
{
	int *ret;
	int i;

	for (i = 0; i < nthreads; i++) {
		pthread_join(threads[i], (void *)&ret);
		assert(ret != NULL);
		assert(*ret == 0);
		free(ret);
	}

	free(threads);
}

int
thread_setup(struct resource *resource ,int num_producers, int num_consumers,
    pthread_t **threadsp)
{
	pthread_t *threads;
	int nthreads = 0;
	int error;
	int i;

	threads = malloc(sizeof(*threads) * (num_producers + num_consumers));
	if (threads == NULL) {
		perror("malloc");
		return (ENOMEM);
	}

	for (i = 0; i < num_consumers; i++) {
		error = pthread_create(&threads[nthreads], NULL, consume, (void *)resource);
		if (error != 0) {
			fprintf(stderr, "pthread_create: %s\n", strerror(error));
			goto error;
		}

		nthreads += 1;
	}

	for (i = 0; i < num_producers; i++) {
		error = pthread_create(&threads[nthreads], NULL, produce, (void *)resource);
		if (error != 0) {
			fprintf(stderr, "pthread_create: %s\n", strerror(error));
			goto error;
		}

		nthreads += 1;
	}

	*threadsp = threads;

	return (0);

error:

	thread_teardown(threads, resource, nthreads);
	return (error);
}

int
main(int argc, char **argv)
{
	long num_producers, num_consumers, ratio;
	struct resource* resource;
	pthread_t *threads;
	int error;

	if (argc != 4) {
		fprintf(stderr, "Usage: ./a.out <# consumers> <# producers> <ratio>\n");
		exit(0);
	}

	num_consumers = strtol(argv[1], NULL, 10);
	num_producers = strtol(argv[2], NULL, 10);
	ratio = strtol(argv[3], NULL, 10);

	resource = resource_setup(num_consumers, num_producers, ratio);
	if (resource == 0) {
		fprintf(stderr, "Failed to acquire resource\n");
		exit(0);
	}

	error = thread_setup(resource, num_producers, num_consumers, &threads);
	if (error != 0) {
		fprintf(stderr, "Failed to set up threads\n");
		resource_teardown(resource);
		exit(0);
	}

	sleep(5);
	set_done(true);
	pthread_mutex_lock(&resource->mutex);
	resource->num_producers = num_consumers;
	pthread_mutex_unlock(&resource->mutex);

	/* Mark operation as done. */

	/* Free the resources. */
	thread_teardown(threads, resource, num_producers + num_consumers);
	resource_teardown(resource);

	return (0);
}
