/*
Assignment 6
Elijah Delavar
CS 360
10/24/2023

Compiling:
	gcc -o assignment6 assignment6.c -lm -lpthread

Running:
	./assignment6
*/

#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#define eatGoal 100			// The least amount of time each philosopher will eat for
#define numPhilosophers 5

// Philosopher structure passed into threads
typedef struct PHIL {
	int ID;
	int *chopsticks;
	pthread_mutex_t *mutex;
} phil;

/*
Get the current time in microseconds.

Courtesy of w1k1n9cc at https://gist.github.com/w1k1n9cc/012be60361e73de86bee0bce51652aa7
*/
long getMicrotime() {
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

/* 	
Successive calls to randomGaussian produce integer return values
having a gaussian distribution with the given mean and standard
deviation.  Return values may be negative.
*/
int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double)mean;
	double sigma = fabs((double)stddev);
	double f1 = sqrt(-2.0 * log((double)rand() / (double)RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double)rand() / (double)RAND_MAX;
	if (rand() & (1 << 5))
		return (int)floor(mu + sigma * cos(f2) * f1);
	else
		return (int)floor(mu + sigma * sin(f2) * f1);
}

/*
Philosopher thinks for a random amount of time.
*/
void think(int ID, int total) {
	int tt; // Sleep time
	tt = randomGaussian(11, 7);
	if (tt < 0) tt = 0;
	printf("Philosopher %d thinking for %d seconds (total = %d)\n", ID, tt, total);
	sleep(tt);
}

/*
Block until pickUpChopsticks is unlocked and this philosopher can pick up both chopsticks.
*/
int waitForChopsticks(int ID, int *chopsticks, pthread_mutex_t *pickUpChopsticks, int rr) {
	while (1) {
		// Wait for access to pick up chopsticks
		if (pthread_mutex_lock(pickUpChopsticks)) {
			fprintf(stderr, "Error with Philosopher %d: Errno %d: %s\n", ID, errno, strerror(errno));
			return 1;
		}

		// Check if philosopher's left and right chopsticks are available
		if (!chopsticks[ID] || !chopsticks[rr]) {
			if (pthread_mutex_unlock(pickUpChopsticks)) {
				fprintf(stderr, "Error with Philosopher %d: Errno %d: %s\n", ID, errno, strerror(errno));
				return 1;
			}
			continue;
		}

		return 0;
	}
	return 1;
}

/*
Philosopher eats for a random amount of time.
Adds time eating to total.
Must pick up both chopsticks adjacent to philosopher in order to start eating.
*/
int eat(int ID, int *chopsticks, pthread_mutex_t *pickUpChopsticks, int *total) {
	int tt; // Sleep time
	int rr; // Right chopstick index

	if (ID-1 < 0) {
		rr = numPhilosophers-1;
	} else {
		rr = ID-1;
	}
	
	// Wait for chopsticks to become available
	printf("Philosopher %d waiting to eat...\n", ID);
	if (waitForChopsticks(ID, chopsticks, pickUpChopsticks, rr)) {
		return 1;
	}

	// Pick up chopsticks
	chopsticks[ID] = 0;
	chopsticks[rr] = 0;

	// Release access to pick up chopsticks
	if (pthread_mutex_unlock(pickUpChopsticks)) {
		fprintf(stderr, "Error with Philosopher %d: Errno %d: %s\n", ID, errno, strerror(errno));
		return 1;
	}

	// Eat
	tt = randomGaussian(9, 3);
	if (tt < 0) tt = 0;
	printf("Philosopher %d eating for %d seconds (total = %d)\n", ID, tt, *total);
	sleep(tt);

	// Put down chopsticks
	chopsticks[ID] = 1;
	chopsticks[rr] = 1;

	*total += tt;
	return 0;
}

/*
Called by philosopher.
Cycles between thinking and eating for at least 100 seconds.

@param pp contains info about philosopher (ID, chopsticks, mutex)
*/
int thinkAndEat(phil *pp) {
	// Goal is to eat for at least 100 seconds
	int total;

	// Setup //
	total = 0;
	srand(getMicrotime());

	// Loop //
	while (total < eatGoal) {
		think(pp->ID, total);
		if (eat(pp->ID, pp->chopsticks, pp->mutex, &total)) {
			return 1;
		}
	}

	// Finished Eating //
	printf("Philosopher %d Exiting...\n", pp->ID);
	return 0;
}

pthread_mutex_t *initMutex() {
	pthread_mutex_t *mutex = NULL;
	mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if (!mutex) {
		fprintf(stderr, "Errno %d: %s\n", errno, strerror(errno));
		return NULL;
	}

	if (pthread_mutex_init(mutex, NULL)) {
		fprintf(stderr, "Errno %d: %s\n", errno, strerror(errno));
		return NULL;
	}
	return mutex;
}

int *initChopsticks() {
	int *chopsticks = NULL;
	chopsticks = (int*)malloc(sizeof(int)*numPhilosophers);
	if (!chopsticks) {
		fprintf(stderr, "Errno %d: %s\n", errno, strerror(errno));
		return NULL;
	}

	for (int ii = 0; ii < numPhilosophers; ii++) {
		chopsticks[ii] = 1;
	}
	
	return chopsticks;
}

/*
Create philosopher threads.
One mutex controls which thread can modify available chopsticks.
*/
void createPhilosophers(pthread_t *threads, int *chopsticks, pthread_mutex_t *pickUpChopsticks, phil *ps) {
	for (int ii = 0; ii < numPhilosophers; ii++) {
		ps[ii].ID = ii;
		ps[ii].chopsticks = chopsticks;
		ps[ii].mutex = pickUpChopsticks;
		pthread_create(threads+ii, NULL, (void*)thinkAndEat, (void*)(ps+ii));
	}
}

/*
Process waits for all threads to return before exiting.
*/
void processWait(pthread_t *threads) {
	for (int ii = 0; ii < numPhilosophers; ii++) {
		pthread_join(threads[ii], NULL);
	}

	puts("Process Exiting...");
}

int main(int argc, char* argv[]) {
	pthread_t threads[numPhilosophers];
	pthread_mutex_t *pickUpChopsticks;	// allocated on heap
	phil ps[numPhilosophers];
	int *chopsticks;					// allocated on heap

	if (!(pickUpChopsticks = initMutex())) {
		exit(1);
	}
	if (!(chopsticks = initChopsticks())) {
		free(pickUpChopsticks);
		exit(1);
	}
	createPhilosophers(threads, chopsticks, pickUpChopsticks, ps);
	processWait(threads);

	free(pickUpChopsticks);
	free(chopsticks);
	return 0;
}