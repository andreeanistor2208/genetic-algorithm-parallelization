#ifndef THREAD_META_DATA_H
#define THREAD_META_DATA_H

struct threadMetaData
{
	sack_object *objects;
	int object_count;
	int generations_count;
	int sack_capacity;
	int thread_count;
	int id;

	individual* current_generation;
	individual* next_generation;
	pthread_barrier_t *pbarrier;
};

#endif
