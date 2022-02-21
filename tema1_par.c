#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include "genetic_algorithm.h"
#include "thread_meta_data.h"


int main(int argc, char *argv[])
 {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	// number of thread
	int thread_count = 0;

	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &thread_count, argc, argv))
		return 0;
	
	// creez vectorul de fire de executie
	pthread_t *threads;
	threads = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
	
	// creez bariera
	pthread_barrier_t barrier;
	pthread_barrier_init(&barrier, NULL, thread_count);

	// creez obiectele de tipul structura pe care sa il dau ca argument functiei paralele
	// pentru a avea tot ce ne trebuie fara variable globale
	struct threadMetaData *smd;
	smd = (struct threadMetaData*)malloc(thread_count * sizeof(struct threadMetaData));

	individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));
	
	// pornesc si opresc o singura data threadurile	
	for (int id = 0; id < thread_count; id++) 
	{
		smd[id].objects = objects;
		smd[id].object_count = object_count;
		smd[id].generations_count = generations_count;
		smd[id].sack_capacity = sack_capacity;
		smd[id].thread_count = thread_count;
		smd[id].id = id;
		smd[id].current_generation = current_generation;
		smd[id].next_generation = next_generation;
		smd[id].pbarrier = &barrier;
		
    	int r = pthread_create(&threads[id], NULL, run_genetic_algorithm, &smd[id]);

    	if (r) {
	  		printf("Eroare la crearea thread-ului %d\n", id);
	  		exit(-1);
		}	
    }

	for (int id = 0; id < thread_count; id++) 
		pthread_join(threads[id],NULL);

	//run_genetic_algorithm(objects, object_count, generations_count, sack_capacity, thread_count);

	free(objects);
	free(threads);
	free(smd);

	// free resources for old generation
	free_generation(current_generation);
	free_generation(next_generation);

	// free resources
	free(current_generation);
	free(next_generation);

	// distrug bariera
	pthread_barrier_destroy(&barrier);

	return 0;
}
