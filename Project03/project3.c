/*
*CS4348.501
*Lam Phan
*Project 3
*/
#include <pthread.h>
#include<semaphore.h>
#include<stdio.h>
#include<unistd.h>
#include <stdlib.h>
#define MAX_IDLE_TIME 3
pthread_mutex_t mutex_lock;
sem_t stud_sem; 
sem_t tutor_sem; 
int waiting_students;
int empty_chair,next_stud;
int occupied=0;
int NUM_OF_STUDENTS;
int NUM_OF_TUTORS; 
int NUM_OF_CHAIRS; 
int NUM_OF_HELPS; 

void* tutor(void *thread_id)
{
	int help_time;
	long t_id = thread_id;
	while(1)
	{
		//Tutor waiting for student
		sem_wait(&stud_sem); 
		help_time = rand() % MAX_IDLE_TIME   + 1;
		pthread_mutex_lock(&mutex_lock);
		printf("\nTutor %ld helping student for %d seconds.",t_id, help_time);
		printf(" Waiting students = %d .",occupied);
		occupied--;
		next_stud = (next_stud + 1)%NUM_OF_CHAIRS;
		pthread_mutex_unlock(&mutex_lock);
		sleep(help_time);
		//Tutor freetime
		sem_post(&tutor_sem); 
	}
	pthread_exit(NULL);
}

void* student(void *thread_id)
{
	int chairs[NUM_OF_CHAIRS];
	long id=(long)thread_id;
	int help_count=0;
	int coding_time;
	int can_wait;
	while(1)
	{
		coding_time= rand() % MAX_IDLE_TIME   + 1;
		sleep(coding_time);
		pthread_mutex_lock(&mutex_lock);
		//Check if all chairs are occupied
		if(occupied==NUM_OF_CHAIRS) 
		{
			printf("\nStudent %ld found no empty chair and will try later.",id);
			pthread_mutex_unlock(&mutex_lock);
			continue;
		}
		else
		{
			printf("\nStudent %ld takes a seat.",id);
			chairs[empty_chair]=id;
			occupied++;
			printf(" Waiting students = %d .",occupied);
			empty_chair = (empty_chair + 1)%NUM_OF_CHAIRS;
			pthread_mutex_unlock(&mutex_lock);
			//Tell tutor that a student is waiting for help
			sem_post(&stud_sem);
			pthread_mutex_unlock(&mutex_lock);
			//Now wait for Tutor to help
			sem_wait(&tutor_sem); 
			help_count++;
			//Check number of help
			if(help_count==NUM_OF_HELPS) 
			break;
		}
	}
	pthread_exit(NULL);
}

int main(int argc, const char *argv[])
{
	//Initialize
	sem_init(&tutor_sem,1,0);
	sem_init(&stud_sem,1,0);
	pthread_mutex_init(&mutex_lock,NULL);
	empty_chair=0;
	next_stud=0;
	NUM_OF_STUDENTS = atoi(argv[1]);
	NUM_OF_TUTORS = atoi(argv[2]);
	NUM_OF_CHAIRS = atoi(argv[3]);
	NUM_OF_HELPS = atoi(argv[4]);
	//Create threads
	pthread_t tutor_thread[NUM_OF_TUTORS],student_thread[NUM_OF_STUDENTS];
	int i=1;
	for(i=1;i<=NUM_OF_TUTORS;i++)
	{
		pthread_create(&tutor_thread[i-1],NULL,tutor, (void *)i);
	}
	
	for(i=1;i<=NUM_OF_STUDENTS;i++)
	{
		pthread_create(&student_thread[i-1],NULL,student,(void *)i);
	}

	for(i=1;i<=NUM_OF_STUDENTS;i++)
	{
		pthread_join(student_thread[i-1],NULL);
	}
	pthread_cancel(tutor_thread);
}