#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "genetic_algorithm.h"
#include "thread_meta_data.h"

int read_input(sack_object **objects, int *object_count, int *sack_capacity, 
				int *generations_count, int *thread_count, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 3) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int)strtol(argv[2], NULL, 10);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*thread_count = (int)strtol(argv[3], NULL, 10);

	if (*thread_count == 0) {
		free(tmp_objects);
		return 0;	
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity)
{
	int weight;
	int profit;

	for (int i = 0; i < object_count; ++i) {
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	int i;
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {
		int first_count = 0, second_count = 0;

		for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i) {
			first_count += first->chromosomes[i];
			second_count += second->chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step)
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
}

void crossover(individual *parent1, individual *child1, int generation_index, int debug) //debug e ptr afisari, va fi scos la final
{
	if (parent1 == NULL || child1 == NULL)
	{
		printf("A dat de NULL");
		return;
	}

	//printf("#%d Intra in crossover pentru generatia %d\n", debug, generation_index);
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	if (parent2 == NULL || child2 == NULL)
	{
		printf("A dat de NULL");
		
	}
	int count = 1 + generation_index % parent1->chromosome_length;
	//printf("#%d count = %d\n", debug, count);

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
	

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	//printf("#%d Al treilea memcpy\n", debug);
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
	//printf("#%d Iese din crossover\n", debug);
}

void copy_individual(const individual *from, const individual *to)
{
	if (from == NULL || to == NULL)
	{
		printf("A dat de NULL(1)\n");
		return;
	}

	if (from->chromosomes == NULL)
	{
		printf("A dat de NULL(2)\n");
		return;
	}
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	if (generation == NULL)
		return;

	int i;

	for (i = 0; i < generation->chromosome_length; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

// MERGE SIMPLU
void merge(individual* a, int low, int mid, int high)
{
	// n1 is size of left side and n2 is size of right side
	int n1 = mid - low + 1;
	int n2 = high - mid;

	individual* left = (individual*)malloc(n1 * sizeof(individual));
	individual* right = (individual*)malloc(n2 * sizeof(individual));

	// storing values in left part
	for (int i = 0; i < n1; i++)
		left[i] = a[i + low];

	// storing values in right part
	for (int i = 0; i < n2; i++)
		right[i] = a[i + mid + 1];

	int k = low;
	int i, j;
	i = j = 0;

	// merge left and right in ascending order
	while (i < n1 && j < n2)
	{
		if (cmpfunc(&left[i], &right[j]) <= 0)
			a[k++] = left[i++];
		else
			a[k++] = right[j++];
	}

	// insert remaining values from left
	while (i < n1)
		a[k++] = left[i++];

	// insert remaining values from right
	while (j < n2)
		a[k++] = right[j++];

	free(left);
	free(right);
}

// MERGE SORT SIMPLU, PRIMUL
void merge_sort(individual* a, int low, int high)
{

	int mid = low + (high - low) / 2;

	if (low < high)
	{
		merge_sort(a, low, mid);
		merge_sort(a, mid + 1, high);
		merge(a, low, mid, high);
	}
}

//void run_genetic_algorithm(const sack_object *objects, int object_count, 
//				int generations_count, int thread_count, int sack_capacity)
void* run_genetic_algorithm(void* arg)
{
	if (arg == NULL)
	{
		printf("Argument with metadata to thread function is NULL\n");
		pthread_exit(NULL);
	}

	int count, cursor;

	// pointer prin care voi accesa ce imi trebuie din metadata
	struct threadMetaData* pSMD = (struct threadMetaData*)arg;

	// iau IDul threadului si restul datelor 
	int thread_id = pSMD->id;
	//printf("Intra in run_generic pentru %d\n", thread_id);
	int N = pSMD->object_count;
	int P = pSMD->thread_count;
	int generations_count = pSMD->generations_count;
	const sack_object *objects = pSMD->objects;
	int sack_capacity = pSMD->sack_capacity;
	pthread_barrier_t* barrier = pSMD->pbarrier;

	individual *current_generation = pSMD->current_generation;
	individual *next_generation = pSMD->next_generation;
	individual *tmp = NULL;

	int start, end; // intervalul de indivizi pe care va rula fiecare thread la crearea primei generatii
					// si alte operatii similare
	start = thread_id * (double)N / P;
	end = fmin((thread_id + 1) * (double)N / P, N);

	// set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = start; i < end; i++) {
		current_generation[i].fitness = 0;
		current_generation[i].chromosomes = (int*) calloc(N, sizeof(int));
		current_generation[i].chromosomes[i] = 1;
		current_generation[i].index = i;
		current_generation[i].chromosome_length = N;

		next_generation[i].fitness = 0;
		next_generation[i].chromosomes = (int*)calloc(N, sizeof(int));
		next_generation[i].index = i;
		next_generation[i].chromosome_length = N;
	}

	//printf("#%d A terminat cu prima generatie start = %d, end = %d\n", thread_id, start, end);
	
	pthread_barrier_wait(barrier);

	// iterate for each generation
	for (int k = 0; k < generations_count; ++k) {
		cursor = 0;
		//printf("#%d Generatia %d\n", thread_id, k);
		// compute fitness and sort by it
		// only the first thread will do it
		if(thread_id == 0)
		{
			compute_fitness_function(objects, current_generation, N, sack_capacity);
			//printf("fitness: ");
			//for (int i = 0;  i < N; i++)
			//	printf("%d ", current_generation[i].fitness);
			//printf("\n");
		}
		
		pthread_barrier_wait(barrier); //asteapta sa calc primul thread val de fitness

		
		//qsort(current_generation, N, sizeof(individual), cmpfunc); // posibil sa o inlocuiesc cu o sortare
																				  // paralelizate

		// MERGE SORT SIMPLU PARALELIZAT
		//int len = N / P;
		//printf("#%d len = %d\n", thread_id, len);
		int start_m = thread_id * (double) N / P;
		int end_m = fmin((thread_id + 1) * (double) N / P - 1, N - 1); // aici sa nu piarda ultimele
		int mid = start_m + (end_m - start_m) / 2;
		if (start_m < end_m)
		{
			//printf("#%d Apel merge_sort %d - %d\n", thread_id, start_m, mid);
			merge_sort(current_generation, start_m, mid);
			//printf("#%d Apel merge sort %d - %d\n", thread_id, mid + 1, end_m);
			merge_sort(current_generation, mid + 1, end_m);
			//printf("#%d Apel merge %d %d %d\n", thread_id, start_m, mid, end_m);
			merge(current_generation, start_m, mid, end_m);

			// afisez valorile de ftiness
			//printf("#%d Valorile sunt:", thread_id);
			//for (int i = start_m; i <= end_m; i++)
			//	printf("%d ", current_generation[i].fitness);
			//printf("\n");
		}

		int v = P; // avem un vector rezultat pentru fiecare thread
		pthread_barrier_wait(barrier);
		// FUZIONARI
		while (v > 1) //avem mai mult de un vector rezultat
		{
			//int len = N / v; //lungimea unui vector rezultat

			// doar threadurile de la 0 la v / 2   vor lucra la interclasari
			if (thread_id < v / 2)
			{
				int start_f = 2 * thread_id * (double) N / v;
				// int end_1 = (2 * thread_id + 1) * (double) N / v - 1;
				int end_f = fmin( (2 * thread_id + 2) * (double) N / v - 1, N - 1);
				//int end_f = fmin( (thread_id + 2) * (double) N /v - 1, N - 1);
				
				int mid_f = start_f + (end_f - start_f) / 2;
				//int mid_f = (2 * thread_id + 1) * (double) N / P - 1;
				//printf("#%d Apel FUZIUNE merge %d %d %d\n", thread_id, start_f, mid_f, end_f);
				merge(current_generation, start_f, mid_f, end_f);
				//printf("#%d Valorile sunt:", thread_id);
				//for (int i = start_f; i <= end_f; i++)
				//	printf("%d ", current_generation[i].fitness);
				//printf("\n");

			}
		
			// daca am avut nr impar de vectori, ultimul thread care a participat la interclasare va face interclasarea
			// si cu ultimul
			if (v % 2 && thread_id == v / 2 - 1)
			{
				int start_f = 2 * thread_id * (double) N / v;
				int end_f = fmin( (2 * thread_id + 3) * (double) N / v - 1, N - 1);
				int mid_f = (2 * thread_id + 2) * (double) N / v - 1;
				//printf("#%d Apel FUZIUNE merge %d %d %d\n", thread_id, start_f, mid_f, end_f);
				merge(current_generation, start_f, mid_f, end_f);
				//printf("#%d Valorile sunt:", thread_id);
				//for (int i = start_f; i <= end_f; i++)
				//	printf("%d ", current_generation[i].fitness);
				//printf("\n");

			}

			// nr de vectori ramasi pentru interclasare			
			v /= 2;
			
			pthread_barrier_wait(barrier);
		}
		
		
		// toti asteapta ca  sa sorteze
		pthread_barrier_wait(barrier);

		//printf("#%d Trece de sortare\n", thread_id);
		// keep first 30% children (elite children selection)
		// o sa impartim aia 30 la suta dintre copii la cele N threaduri
		count = N * 3 / 10;
		int start_Elite, end_Elite; // intervalul de indivizi pe care va rula fiecare thread
		start_Elite = thread_id * (double)count / P;
		end_Elite = fmin((thread_id + 1) * (double)count / P, count);
		//printf("#%d Se pregateste de copy_30%% elite cu start = %d, end = %d\n", thread_id, start_Elite, end_Elite);
		for (int i = start_Elite; i < end_Elite; i++) 
			copy_individual(current_generation + i, next_generation + i);
		
		cursor = count;
		//printf("#%d PAstrez primii 30%% cu start = %d, end = %d, cursor va deveni %d\n", thread_id, start_Elite, end_Elite, cursor);
		
		pthread_barrier_wait(barrier); // posibil sa scot aceasta bariera?

		// mutate first 20% children with the first version of bit string mutation
		count = N * 2 / 10;
		// again we divide the first 20% children to the N threads
		int start_mutate_1, end_mutate_1;
		start_mutate_1 = thread_id * (double)count / P;
		end_mutate_1 = fmin((thread_id + 1) * (double)count / P, count);

		for (int i = start_mutate_1; i < end_mutate_1; i++) 
		{
			copy_individual(current_generation + i, next_generation + cursor + i);
			mutate_bit_string_1(next_generation + cursor + i, k);
		}
		cursor += count;
		

		for (int i = start_mutate_1; i < end_mutate_1; i++) 
		{
			//printf("#%d ma pregatesc de copy_individual, i + count = %d, cursor + i = %d\n", thread_id, count + i, cursor + i);
			copy_individual(current_generation + i + count, next_generation + cursor + i);
			//printf("#%d ma pregatesc de mutate_bit_string_2\n", thread_id);
			mutate_bit_string_2(next_generation + cursor + i, k);
		}
		cursor += count;
		//printf("#%d Mutatia a 2 a urmatorilor 20%% cu start = %d, end = %d, cursor va deveni %d\n", thread_id, start_mutate_1, end_mutate_1, cursor);
		pthread_barrier_wait(barrier);

		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)
		count = N * 3 / 10;
		int start_cross, end_cross;
		start_cross = thread_id * (double)count / P;
		end_cross = fmin((thread_id + 1) * (double)count / P, count);
		
		if (thread_id == 0 && count % 2 == 1) // asta cred ca face doar thread 0 
		 {
			copy_individual(current_generation + N - 1, next_generation + cursor + count - 1);
			end_cross--;
		}
		else if (count % 2 == 1)
			end_cross--;

		//printf("#%d start_cross = %d, end_cross = %d, cursor = %d\n", thread_id, start_cross, end_cross, cursor);

		for (int i = start_cross; i < end_cross; i += 2)
			crossover(current_generation + i, next_generation + cursor + i, k, thread_id);
		
		
		pthread_barrier_wait(barrier); // oare merge scos?
		
		// switch to new generation
		tmp = current_generation;
		current_generation = next_generation;
		next_generation = tmp; // de ce face asta?

		for (int i = start; i < end; i++)
			current_generation[i].index = i;

		

		if (thread_id == 0 && k % 5 == 0) //asta va face doar thread 0
			print_best_fitness(current_generation); // de vazut ce face asta

		pthread_barrier_wait(barrier);
		
	} // aici termina de frecat generatiile

	if (thread_id == 0)
	{
		compute_fitness_function(objects, current_generation, N, sack_capacity); //doar un thread
		qsort(current_generation, N, sizeof(individual), cmpfunc); //inlocuit iar cu sortare paralela
		print_best_fitness(current_generation); // doar un thread
	}
	

	pthread_exit(NULL);
}
