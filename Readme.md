# Assignment 6 - Dining Philosophers

### CS 360

### Elijah Delavar

### 10/24/2023

## Execution

To compile:

	$ gcc -o assignment6 assignment6.c -lm -lpthread

To run:

    $ ./assignment6.c

## Description

This program creates five "philosopher" threads.  Each philosopher is sat in between two "chopstick" integers, totaling 5 chopsticks.  Each philosopher must think and eat at random intervals until all have eaten for at least 100 seconds.  Each philosopher must have exactly two chopsticks in order to eat.  

A mutex controls which thread can approach the table to pick up chopsticks.  If a thread is unable to approach the table, it waits until both chopsticks are available.
