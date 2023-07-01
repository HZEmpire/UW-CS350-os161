#include "assignment.h"
#include <stdio.h>

void
consume_enter(struct resource *resource)
{    
    // FILL ME IN
    // Lock the mutex
    pthread_mutex_lock(&resource->mutex);

    // wait until there are no producers
    while (resource->num_producers * resource->ratio <= resource->num_consumers) {
        pthread_cond_wait(&resource->cond, &resource->mutex);
    }

    // increment the number of consumers with synchronized access
    resource->num_consumers += 1;
    //pthread_mutex_unlock(&resource->mutex); has been called in orderme.c
}

void
consume_exit(struct resource *resource)
{   
    // FILL ME IN
    // decrement the number of consumers with synchronized access
    //pthread_mutex_lock(&resource->mutex); has been called in orderme.c
    resource->num_consumers -= 1;

    // Wake up any waiting producers or consumers
    if (resource->num_producers * resource->ratio > resource->num_consumers){
        pthread_cond_signal(&resource->cond);
    }
    pthread_mutex_unlock(&resource->mutex);
}

void
produce_enter(struct resource *resource)
{   
    // FILL ME IN
    // increment the number of producers with synchronized access
    pthread_mutex_lock(&resource->mutex);
    resource->num_producers += 1;

    // Wake up any waiting producers or consumers
    if (resource->num_producers * resource->ratio > resource->num_consumers){
        pthread_cond_signal(&resource->cond);
    }
    //pthread_mutex_unlock(&resource->mutex); has been called in orderme.c
}

void
produce_exit(struct resource *resource)
{   
    // FILL ME IN
    // Lock the mutex
    //pthread_mutex_lock(&resource->mutex); has been called in orderme.c
    
    // wait until there (resource->num_producers -1) * resource->ratio >= resource->num_consumers
    while ((resource->num_producers - 1) * resource->ratio < resource->num_consumers) {
        pthread_cond_wait(&resource->cond, &resource->mutex);
    }

    // decrement the number of producers with synchronized access
    resource->num_producers -= 1;

    pthread_mutex_unlock(&resource->mutex);
}


