#include "queue.c"
#include "string.h"
#include <stdint.h>	 // uint32
#include <stdlib.h>	 // exit, perror
#include <unistd.h>	 // usleep
#include <stdio.h>	 // printf
#include <pthread.h> // pthread_
#include <sys/time.h>
#include <time.h>

int simulationTime = 1200;	 // simulation time
int seed = 10;				 // seed for randomness
int emergencyFrequency = 30; // frequency of emergency gift requests from New Zealand

void *ElfA(void *arg); // the one that can paint
void *ElfB(void *arg); // the one that can assemble
void *Santa(void *arg);
void *ControlThread(void *arg); // handles printing and queues (up to you)

#define THREAD_NUM 4
int counter = 0;

Queue *packagingReq;
Queue *paintingReq;
Queue *assemblyReq;
Queue *deliveryReq;
Queue *qaReq;
Queue *waitingQ;

pthread_mutex_t mutexPac;
pthread_mutex_t mutexPai;
pthread_mutex_t mutexAs;
pthread_mutex_t mutexDel;
pthread_mutex_t mutexQA;
pthread_mutex_t mutexFile;
pthread_mutex_t mutexWaiting;

struct timeval start_time;
struct timeval current_time;

int TaskID = 0;
int pthread_sleep(int seconds)
{
	pthread_mutex_t mutex;
	pthread_cond_t conditionvar;
	struct timespec timetoexpire;
	if (pthread_mutex_init(&mutex, NULL))
	{
		return -1;
	}
	if (pthread_cond_init(&conditionvar, NULL))
	{
		return -1;
	}

	struct timeval tp;
	// When to expire is an absolute time, so get the current time and add it to our delay time
	gettimeofday(&tp, NULL);
	timetoexpire.tv_sec = tp.tv_sec + seconds;
	timetoexpire.tv_nsec = tp.tv_usec * 1000;

	// int a= pthread_mutex_lock(&mutex);
	// pthread_mutex_unlock(&mutex);
	pthread_mutex_lock(&mutex);
	int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
	// pthread_cond_signal(&conditionvar);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&conditionvar);
	// printf("pthread_cond_timedwait %d\n",res);
	// Upon successful completion, a value of zero shall be returned
	return res;
}

int main(int argc, char **argv)
{
	// -t (int) => simulation time in seconds
	// -s (int) => change the random seed
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-t"))
		{
			simulationTime = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-s"))
		{
			seed = atoi(argv[++i]);
		}
	}

	gettimeofday(&start_time, NULL);
	// struct timeval current_time;
	gettimeofday(&current_time, NULL);
	srand(seed);

	pthread_t threads[THREAD_NUM];
	printf("IN MAIN\n");
	// Queue usage example
	packagingReq = ConstructQueue(1000);
	paintingReq = ConstructQueue(1000);
	assemblyReq = ConstructQueue(1000);
	deliveryReq = ConstructQueue(1000);
	qaReq = ConstructQueue(1000);

	// waitingPaintingQ = ConstructQueue(1000);
	// waitingAsemblyQ = ConstructQueue(1000);
	waitingQ = ConstructQueue(1000);
	// EVERY QUEUE NEED LOCKS INITIALIZE MUTEXES

	if (pthread_mutex_init(&mutexPac, NULL))
	{
		return -1;
	}

	if (pthread_mutex_init(&mutexPai, NULL))
	{
		return -1;
	}

	if (pthread_mutex_init(&mutexAs, NULL))
	{
		return -1;
	}

	if (pthread_mutex_init(&mutexDel, NULL))
	{
		return -1;
	}

	if (pthread_mutex_init(&mutexQA, NULL))
	{
		return -1;
	}

	if (pthread_mutex_init(&mutexWaiting, NULL))
	{
		return -1;
	}

	if (pthread_mutex_init(&mutexFile, NULL))
	{
		return -1;
	}

	FILE *file;
	file = fopen("events.log", "w");
	fprintf(file, "%s", "TID GID GT TTy ReqT TAr TT Res\n");
	fclose(file);

	pthread_create(&threads[0], NULL, ControlThread, NULL);
	pthread_create(&threads[1], NULL, ElfA, NULL);
	pthread_create(&threads[2], NULL, ElfB, NULL);
	pthread_create(&threads[3], NULL, Santa, NULL);

	for (int i = 0; i < THREAD_NUM; i++)
	{
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&mutexPac);
	pthread_mutex_destroy(&mutexPai);
	pthread_mutex_destroy(&mutexAs);
	pthread_mutex_destroy(&mutexDel);
	pthread_mutex_destroy(&mutexQA);
	pthread_mutex_destroy(&mutexWaiting);
	pthread_mutex_destroy(&mutexFile);

	DestructQueue(paintingReq);
	DestructQueue(packagingReq);
	DestructQueue(assemblyReq);
	DestructQueue(qaReq);
	DestructQueue(deliveryReq);
	DestructQueue(waitingQ);

	printf("OUT MAIN\n");
	return 0;
}

void *ElfA(void *arg)
{
	while (start_time.tv_sec + simulationTime+30 > current_time.tv_sec)
	{
		
		// ENTER ELF A AND GET ITS LOCK
		// pthread_mutex_lock(&mutexA);
		int tired = 0;
		int GiftID;
		int GiftType;
		char *TaskType;
		int RequestTime;
		int TaskArrival;
		int TT;
		int prio;

		// GIVE PRIORITY TO THE QUEUE WHICH HAS NEW ZEALANDER
		pthread_mutex_lock(&mutexPai);
		if (ContainsNZ(paintingReq))
		{
			tired = -1;
			printf("I GAVE PRIORITY TO PAINTING FOR PART 3\n");
		}
		pthread_mutex_unlock(&mutexPai);

		pthread_mutex_lock(&mutexPac);
		if (ContainsNZ(packagingReq))
		{
			tired = 0;
			printf("I GAVE PRIORITY TO PACKAGING FOR PART 3\n");
		}
		pthread_mutex_unlock(&mutexPac);

		// do packaging
		pthread_mutex_lock(&mutexPac); // TO CHECK PACKAGING QUEUE GET QUEUE LOCK
		if (!isEmpty(packagingReq) && tired != -1)
		{										 // CHECK PACKAGE QUEUE
			Task packed = Dequeue(packagingReq); // DO THE PACKAGING
			GiftID = packed.ID;
			GiftType = packed.type;
			TaskType = "C";
			RequestTime = packed.reqTime;
			TaskArrival = packed.taskArr;
			prio = packed.prioritize;
			printf("ELF A DID PACKAGING FOR ID: %d OF TYPE: %d \n", packed.ID, packed.type);
			tired = 1;
			pthread_mutex_unlock(&mutexPac); // RELEASE PACKAGE QUEUE LOCK
			// printf("ELF A SLEEPS 1\n");
			// pthread_sleep(1); // TAKE YOUR TIME
			printf("ELF A SLEEPS 1\n");
			pthread_sleep(1);			   // TAKE YOUR TIME
			pthread_mutex_lock(&mutexDel); // TO ENQUEU TO DELIVERY QUEUE GET ITS LOCK
			gettimeofday(&current_time, NULL);
			packed.taskArr = current_time.tv_sec - start_time.tv_sec;
			Enqueue(deliveryReq, packed); // SEND PACKED GIFT TO DELIVERY
			printf("ELF A SEND TO DELIVERY ID: %d OF TYPE: %d \n", packed.ID, packed.type);
			pthread_mutex_unlock(&mutexDel); // RELEASE DELIVERY LOCK
			// DONE PACKAGING
			pthread_mutex_lock(&mutexFile);
			FILE *file;
			file = fopen("events.log", "a+");
			gettimeofday(&current_time, NULL);
			TaskID = TaskID + 1;
			TT = current_time.tv_sec - TaskArrival - start_time.tv_sec;
			// TaskID GiftID GiftType TaskType RequestTime TaskArrival TT Responsible
			fprintf(file, "%d\t %d\t %d\t %s\t %d\t %d\t %d\t %s\t prioritize:%d\t\n", TaskID, GiftID, GiftType, TaskType, RequestTime, TaskArrival, TT, "A", prio);
			fclose(file);
			pthread_mutex_unlock(&mutexFile);
		}
		else
		{
			pthread_mutex_unlock(&mutexPac); // RELEASE PACKAGE QUEUE LOCK
		}

		// do painting
		if (tired != 1)
		{
			pthread_mutex_lock(&mutexPai); // GET THE LOCK OF PAINTING QUEUE
			if (!isEmpty(paintingReq))
			{
				Task painted = Dequeue(paintingReq); // DO PAINTING
				GiftID = painted.ID;
				GiftType = painted.type;
				TaskType = "P";
				RequestTime = painted.reqTime;
				TaskArrival = painted.taskArr;
				prio = painted.prioritize;
				printf("ELF A DID PAINTING FOR ID: %d OF TYPE: %d \n", painted.ID, painted.type);
				tired = 2;
				pthread_mutex_unlock(&mutexPai); // GET THE LOCK OF PAINTING QUEUE
				printf("ELF A SLEEPS 3\n");
				pthread_sleep(3); // TAKE YOUR TIME
				// THERE ARE 2 CASES FOR NEXT TASK
				if (painted.type == 2)
				{ // DIRECT PACKAGING
					pthread_mutex_lock(&mutexPac);
					gettimeofday(&current_time, NULL);
					painted.taskArr = current_time.tv_sec - start_time.tv_sec;
					Enqueue(packagingReq, painted); // SEND TASK TO PACKAGE QUEUE
					printf("ELF A SEND TO PACKAGE ID: %d OF TYPE: %d \n", painted.ID, painted.type);
					pthread_mutex_unlock(&mutexPac);
				}
				else if (painted.type == 4)
				{									   // CHECK SANTA'S QA QUEUE
					pthread_mutex_lock(&mutexWaiting); // GET QA QUEUE LOCK TO CHEK
					if (Contains(waitingQ, painted.ID))
					{										 // CHECK QA QUEUE, IF THERE EXISTS AN INPUT WITH SAME ID SANTA STILL HAS JOB TO DO DO NOT SEND TO PACKAGING
						pthread_mutex_unlock(&mutexWaiting); // RELEASE QA QUEUE
						pthread_mutex_lock(&mutexPac);		 // GET PACKAGING QUEUE
						gettimeofday(&current_time, NULL);
						painted.taskArr = current_time.tv_sec - start_time.tv_sec;
						Enqueue(packagingReq, painted); // SEND TASK TO PACKAGING QUEUE
						printf("ELF A SEND TO PACKAGE ID: %d OF TYPE: %d \n", painted.ID, painted.type);
						pthread_mutex_unlock(&mutexPac); // RELEASE PACKAGING QUEUE
					}
					else
					{
						// gettimeofday(&current_time, NULL);
						// painted.reqTime=current_time.tv_sec;
						Enqueue(waitingQ, painted);			 // SEND TASK TO WAITING QUEUE
						pthread_mutex_unlock(&mutexWaiting); // RELEASE QA QUEUE
					}
				}
				// DONE PAINTING
				pthread_mutex_lock(&mutexFile);
				FILE *file;
				file = fopen("events.log", "a+");
				gettimeofday(&current_time, NULL);
				TaskID = TaskID + 1;
				TT = current_time.tv_sec - TaskArrival - start_time.tv_sec;
				// TaskID GiftID GiftType TaskType RequestTime TaskArrival TT Responsible
				fprintf(file, "%d\t %d\t %d\t %s\t %d\t %d\t %d\t %s\t prioritize:%d\t \n", TaskID, GiftID, GiftType, TaskType, RequestTime, TaskArrival, TT, "A", prio);
				fclose(file);
				pthread_mutex_unlock(&mutexFile);
			}
			else
			{
				pthread_mutex_unlock(&mutexPai); // RELEASE PAINTING QUEUE LOCK
			}

			// printf("released THE LOCK S\n");
		}
		// FINISH UPDATING
		tired = 0;
		// pthread_mutex_unlock(&mutexA); // RELEASE ELF A
		gettimeofday(&current_time, NULL);
	}
}

void *ElfB(void *arg)
{
	while (start_time.tv_sec + simulationTime+30 > current_time.tv_sec)
	{
		
		// pthread_mutex_lock(&mutexB); // GET THE LOCK OF ELF B
		int tired = 0;
		int GiftID;
		int GiftType;
		char *TaskType;
		int RequestTime;
		int TaskArrival;
		int TT;
		int prio;

		// GIVE PRIORITY TO THE QUEUE WHICH HAS NEW ZEALANDER
		pthread_mutex_lock(&mutexAs);
		if (ContainsNZ(assemblyReq))
		{
			tired = -1;
			printf("I GAVE PRIORITY TO ASSEMBLY FOR PART 3\n");
		}
		pthread_mutex_unlock(&mutexAs);

		pthread_mutex_lock(&mutexPac);
		if (ContainsNZ(packagingReq))
		{
			tired = 0;
			printf("I GAVE PRIORITY TO PACKAGING FOR PART 3\n");
		}
		pthread_mutex_unlock(&mutexPac);

		//  do packaging
		pthread_mutex_lock(&mutexPac);
		if (!isEmpty(packagingReq) && tired != -1)
		{
			Task packed = Dequeue(packagingReq); // DO PACKAGING
			GiftID = packed.ID;
			GiftType = packed.type;
			TaskType = "C";
			RequestTime = packed.reqTime;
			TaskArrival = packed.taskArr;
			prio = packed.prioritize;
			printf("ELF B DID PACKAGING FOR ID: %d OF TYPE: %d \n", packed.ID, packed.type);
			tired = 1;
			pthread_mutex_unlock(&mutexPac);
			printf("ELF B SLEEPS 1\n");
			pthread_sleep(1); // TAKE YOUR TIME
			pthread_mutex_lock(&mutexDel);
			gettimeofday(&current_time, NULL);
			packed.taskArr = current_time.tv_sec - start_time.tv_sec;
			Enqueue(deliveryReq, packed); // SEND TO DELIVERY AFTER PACKAGING
			printf("ELF B SEND DELIVERY FOR ID: %d OF TYPE: %d \n", packed.ID, packed.type);
			pthread_mutex_unlock(&mutexDel);

			pthread_mutex_lock(&mutexFile);
			FILE *file;
			file = fopen("events.log", "a+");
			gettimeofday(&current_time, NULL);
			TaskID = TaskID + 1;
			TT = current_time.tv_sec - TaskArrival - start_time.tv_sec;
			// TaskID GiftID GiftType TaskType RequestTime TaskArrival TT Responsible
			fprintf(file, "%d\t %d\t %d\t %s\t %d\t %d\t %d\t %s\t prioritize:%d\t\n", TaskID, GiftID, GiftType, TaskType, RequestTime, TaskArrival, TT, "B", prio);
			fclose(file);
			pthread_mutex_unlock(&mutexFile);
		}
		else
		{
			pthread_mutex_unlock(&mutexPac);
		}

		// do assembly if not tired
		if (tired != 1)
		{ // DO NOT DO JOBS SEQUENTIALLY
			pthread_mutex_lock(&mutexAs);
			if (!isEmpty(assemblyReq))
			{
				Task assembled = Dequeue(assemblyReq); // DO ASSEMBLY JOB
				GiftID = assembled.ID;
				GiftType = assembled.type;
				TaskType = "A";
				RequestTime = assembled.reqTime;
				TaskArrival = assembled.taskArr;
				prio = assembled.prioritize;
				printf("ELF B DID ASSEMBLY FOR ID: %d OF TYPE: %d \n", assembled.ID, assembled.type);
				tired = 2;
				pthread_mutex_unlock(&mutexAs);
				printf("ELF B SLEEPS 2\n");
				pthread_sleep(2); // TAKE YOUR TIME
				if (assembled.type == 3)
				{ // JUST DO ASSEMBLY AND SEND TO PACKAGING
					pthread_mutex_lock(&mutexPac);
					gettimeofday(&current_time, NULL);
					assembled.taskArr = current_time.tv_sec - start_time.tv_sec;
					Enqueue(packagingReq, assembled); // SEND TO PACKAGING AFTER ASSEMBLY
					printf("ELF B SEND PACKAGE FOR ID: %d OF TYPE: %d \n", assembled.ID, assembled.type);
					pthread_mutex_unlock(&mutexPac);
				}
				else if (assembled.type == 5)
				{ // Should check santa
					pthread_mutex_lock(&mutexWaiting);
					if (Contains(waitingQ, assembled.ID))
					{ // CHECK SANTA'S QA QUEUE IF IT DOESNT HAVE ID IT MEANS SANTA DID ITS JOB
						pthread_mutex_unlock(&mutexWaiting);
						pthread_mutex_lock(&mutexPac);
						gettimeofday(&current_time, NULL);
						assembled.taskArr = current_time.tv_sec - start_time.tv_sec;
						Enqueue(packagingReq, assembled); // SEND TO PACKAGING
						printf("ELF B SEND PACKAGE FOR ID: %d OF TYPE: %d \n", assembled.ID, assembled.type);
						pthread_mutex_unlock(&mutexPac);
					}
					else
					{
						Enqueue(waitingQ, assembled);
						pthread_mutex_unlock(&mutexWaiting);
					}
				}
				pthread_mutex_lock(&mutexFile);
				FILE *file;
				file = fopen("events.log", "a+");
				gettimeofday(&current_time, NULL);
				TaskID = TaskID + 1;
				TT = current_time.tv_sec - TaskArrival - start_time.tv_sec;
				// TaskID GiftID GiftType TaskType RequestTime TaskArrival TT Responsible
				fprintf(file, "%d\t %d\t %d\t %s\t %d\t %d\t %d\t %s\t prioritize:%d\t\n", TaskID, GiftID, GiftType, TaskType, RequestTime, TaskArrival, TT, "B", prio);
				fclose(file);
				pthread_mutex_unlock(&mutexFile);
			}
			else
			{
				pthread_mutex_unlock(&mutexAs);
			}
		}
		tired = 0;
		// pthread_mutex_unlock(&mutexB); // RELEASE ELF B
		gettimeofday(&current_time, NULL);
	}
}

// manages Santa's tasks
void *Santa(void *arg)
{
	while (start_time.tv_sec + simulationTime+30 > current_time.tv_sec)
	{
		
		// pthread_mutex_lock(&mutexS);
		int tired = 0;

		int GiftID;
		int GiftType;
		char *TaskType;
		int RequestTime;
		int TaskArrival;
		int TT;
		int prio;
		// PRIORITIZE QA IF DELREQ IS EMPTY (already implemented) OR QA HAS MORE THAN 3 INPUTS

		pthread_mutex_lock(&mutexQA);
		if (qaReq->size >= 3)
		{
			tired = -1;
			printf("I GAVE PRIORITY TO QA FOR PART 2\n");
		}
		pthread_mutex_unlock(&mutexQA);

		// GIVE PRIORITY TO THE QUEUE WHICH HAS NEW ZEALANDER
		pthread_mutex_lock(&mutexDel);
		if (ContainsNZ(deliveryReq))
		{
			tired = 0;
			printf("I GAVE PRIORITY TO DELIVERY FOR PART 3\n");
		}
		pthread_mutex_unlock(&mutexDel);

		pthread_mutex_lock(&mutexQA);
		if (ContainsNZ(qaReq))
		{
			tired = -1;
			printf("I GAVE PRIORITY TO QA FOR PART 3\n");
		}
		pthread_mutex_unlock(&mutexQA);

		// do delivery
		pthread_mutex_lock(&mutexDel);
		if (!isEmpty(deliveryReq) & tired != -1)
		{
			Task delivered = Dequeue(deliveryReq); // DO DELIVERY
			GiftID = delivered.ID;
			GiftType = delivered.type;
			TaskType = "D";
			RequestTime = delivered.reqTime;
			TaskArrival = delivered.taskArr;
			prio = delivered.prioritize;
			printf("SANTA DID DELIVERY FOR ID: %d OF TYPE: %d \n", delivered.ID, delivered.type);
			tired = 1;
			pthread_mutex_unlock(&mutexDel);
			printf("SANTA SLEEPS 1\n");
			pthread_sleep(1); // TAKE YOUR TIME
			pthread_mutex_lock(&mutexFile);
			FILE *file;
			file = fopen("events.log", "a+");
			gettimeofday(&current_time, NULL);
			TaskID = TaskID + 1;
			TT = current_time.tv_sec - TaskArrival - start_time.tv_sec;
			// TaskID GiftID GiftType TaskType RequestTime TaskArrival TT Responsible
			fprintf(file, "%d\t %d\t %d\t %s\t %d\t %d\t %d\t %s\t prioritize:%d\t \n", TaskID, GiftID, GiftType, TaskType, RequestTime, TaskArrival, TT, "S", prio);
			fclose(file);
			pthread_mutex_unlock(&mutexFile);
		}
		else
		{
			pthread_mutex_unlock(&mutexDel);
		}
		// if not tired do qa
		if (tired != 1)
		{
			// do QA
			pthread_mutex_lock(&mutexQA);
			if (!isEmpty(qaReq))
			{
				Task qaed = Dequeue(qaReq); // DO QUALITY CONTROL
				GiftID = qaed.ID;
				GiftType = qaed.type;
				TaskType = "Q";
				RequestTime = qaed.reqTime;
				TaskArrival = qaed.taskArr;
				prio = qaed.prioritize;
				printf("SANTA DID QA FOR ID: %d OF TYPE: %d \n", qaed.ID, qaed.type);
				tired = 2;
				pthread_mutex_unlock(&mutexQA);
				printf("SANTA SLEEPS\n");
				pthread_sleep(1); // TAKE YOUR TIME
				// printf("GONNA DO QA of type %d S\n", qaed.type);
				if (qaed.type == 4)
				{ // CHECK PAINTING
					pthread_mutex_lock(&mutexWaiting);
					if (Contains(waitingQ, qaed.ID))
					{ // CHECK PAINTING QUEUE
						pthread_mutex_unlock(&mutexWaiting);
						pthread_mutex_lock(&mutexPac);
						gettimeofday(&current_time, NULL);
						qaed.taskArr = current_time.tv_sec - start_time.tv_sec;
						Enqueue(packagingReq, qaed); // SEND TO PACKAGING
						printf("SANTA SEND PACKAGE FOR ID: %d OF TYPE: %d \n", qaed.ID, qaed.type);
						pthread_mutex_unlock(&mutexPac);
						// printf("SENT TO PACKING FROM QA S\n");
					}
					else
					{
						Enqueue(waitingQ, qaed);
						pthread_mutex_unlock(&mutexWaiting);
					}
				}
				else if (qaed.type == 5)
				{ // CHECK ASSEMBLY
					pthread_mutex_lock(&mutexWaiting);
					if (Contains(waitingQ, qaed.ID))
					{ // CHECK ASSEMBLY QUEUE
						pthread_mutex_unlock(&mutexWaiting);
						pthread_mutex_lock(&mutexPac);
						gettimeofday(&current_time, NULL);
						qaed.taskArr = current_time.tv_sec - start_time.tv_sec;
						Enqueue(packagingReq, qaed); // IF ASSEMBLY DONE SEND TO PACKAGING
						printf("SANTA SEND PACKAGE FOR ID: %d OF TYPE: %d \n", qaed.ID, qaed.type);
						pthread_mutex_unlock(&mutexPac);
						// printf("SENT TO PACKING FROM QA S\n");
					}
					else
					{
						Enqueue(waitingQ, qaed);
						pthread_mutex_unlock(&mutexWaiting);
					}
				}
				// printf("DONE QA S\n");
				pthread_mutex_lock(&mutexFile);
				FILE *file;
				file = fopen("events.log", "a+");
				gettimeofday(&current_time, NULL);
				TaskID = TaskID + 1;
				TT = current_time.tv_sec - TaskArrival - start_time.tv_sec;
				// TaskID GiftID GiftType TaskType RequestTime TaskArrival TT Responsible
				fprintf(file, "%d\t %d\t %d\t %s\t %d\t %d\t %d\t %s\t prioritize: %d\t \n", TaskID, GiftID, GiftType, TaskType, RequestTime, TaskArrival, TT, "S", prio);
				fclose(file);
				pthread_mutex_unlock(&mutexFile);
			}
			else
			{
				pthread_mutex_unlock(&mutexQA);
			}
		}
		tired = 0;
		// pthread_mutex_unlock(&mutexS); // RELEASE SANTA
		gettimeofday(&current_time, NULL);
	}
}

// the function that controls queues and output
void *ControlThread(void *arg)
{
	// struct timeval start_time;
	// gettimeofday(&start_time, NULL);
	while (start_time.tv_sec + simulationTime > current_time.tv_sec)
	{
		
		// pthread_mutex_lock(&mutexC); // LOCK CONTROLLER

		Task t;
		char kBehave[10] = "";				// KID CATEGORY
		int kRate = (rand() % 100) + 1;		// KID POSSIBILITY
		int ttypeRate = (rand() % 100) + 1; // TOY TYPE ~ WOODEN OR PLASTIC

		if ((current_time.tv_sec - start_time.tv_sec) % emergencyFrequency == 0)
		{					  // PRIORITY TO FAR KIDS
			t.prioritize = 1; // COME AT EVERY 30 T
			printf("THERE IS A NEW ZEALANDER IN LINE\n");
		}
		else
		{
			t.prioritize = 0;
		}
		// if (kRate > 0 && kRate <= 10)
		if (kRate < 0)
		{
			strcpy(kBehave, "bad");
		}
		else if (kRate > 0 && kRate <= 50)
		{
			strcpy(kBehave, "okay");
			t.type = 1;
			t.ID = counter; // 5 * counter + t.type;
			gettimeofday(&current_time, NULL);
			printf("INCOMING REQUEST FROM CONTROLLER Task ID %d, counter: %d, type: %d \n", t.ID, counter, t.type); // INCOMING REQUEST
			pthread_mutex_lock(&mutexPac);
			t.reqTime = current_time.tv_sec - start_time.tv_sec;
			t.taskArr = current_time.tv_sec - start_time.tv_sec;
			Enqueue(packagingReq, t);
			pthread_mutex_unlock(&mutexPac);
			// printf("Task ID %d, counter: %d, type: %d\n",t.ID,counter,t.type);
			// pthread_sleep(1);
			// printf("CONTROLLER ADDED TASK PACKAGE\n");
		}
		else if (kRate > 50 && kRate <= 90)
		{
			strcpy(kBehave, "good");
			if (ttypeRate > 0 && ttypeRate <= 50)
			{ // WOODEN TOY
				t.type = 2;
				t.ID = counter; // 5 * counter + t.type;
				gettimeofday(&current_time, NULL);
				printf("INCOMING REQUEST FROM CONTROLLER Task ID %d, counter: %d, type: %d \n", t.ID, counter, t.type); // INCOMING REQUEST
				pthread_mutex_lock(&mutexPai);
				t.reqTime = current_time.tv_sec - start_time.tv_sec;
				t.taskArr = current_time.tv_sec - start_time.tv_sec;
				Enqueue(paintingReq, t);
				pthread_mutex_unlock(&mutexPai);
				//	printf("Task ID %d, counter: %d, type: %d\n",t.ID,counter,t.type);
				//  	pthread_sleep(1);
				// printf("CONTROLLER ADDED TASK PAINT\n");
			}
			else
			{
				t.type = 3;
				t.ID = counter; // 5 * counter + t.type;
				gettimeofday(&current_time, NULL);
				printf("INCOMING REQUEST FROM CONTROLLER Task ID %d, counter: %d, type: %d \n", t.ID, counter, t.type); // INCOMING REQUEST
				pthread_mutex_lock(&mutexAs);
				t.reqTime = current_time.tv_sec - start_time.tv_sec;
				t.taskArr = current_time.tv_sec - start_time.tv_sec;
				Enqueue(assemblyReq, t);
				pthread_mutex_unlock(&mutexAs);
				// printf("Task ID %d, counter: %d, type: %d\n",t.ID,counter,t.type);
				// pthread_sleep(1);
				// printf("CONTROLLER ADDED TASK ASSEMBLY\n");
			}
		}
		else if (kRate > 90 && kRate <= 100)
		{
			strcpy(kBehave, "excellent");
			if (ttypeRate > 0 && ttypeRate <= 50)
			{ // WOODEN TOY
				t.type = 4;
				t.ID = counter; // 5 * counter + t.type;
				gettimeofday(&current_time, NULL);
				printf("INCOMING REQUEST FROM CONTROLLER Task ID %d, counter: %d, type: %d \n", t.ID, counter, t.type); // INCOMING REQUEST
				pthread_mutex_lock(&mutexPai);
				t.reqTime = current_time.tv_sec - start_time.tv_sec;
				t.taskArr = current_time.tv_sec - start_time.tv_sec;
				Enqueue(paintingReq, t);
				pthread_mutex_unlock(&mutexPai);
				pthread_mutex_lock(&mutexQA);
				t.reqTime = current_time.tv_sec - start_time.tv_sec;
				t.taskArr = current_time.tv_sec - start_time.tv_sec;
				Enqueue(qaReq, t);
				pthread_mutex_unlock(&mutexQA);
				// printf("Task ID %d, counter: %d, type: %d\n",t.ID,counter,t.type);
				// pthread_sleep(1);
				// printf("CONTROLLER ADDED TASK PAINT AND QA\n");
			}
			else
			{ // PLASTIC
				t.type = 5;
				t.ID = counter; // 5 * counter + t.type;
				gettimeofday(&current_time, NULL);
				printf("INCOMING REQUEST FROM CONTROLLER Task ID %d, counter: %d, type: %d \n", t.ID, counter, t.type); // INCOMING REQUEST
				pthread_mutex_lock(&mutexAs);
				t.reqTime = current_time.tv_sec - start_time.tv_sec;
				t.taskArr = current_time.tv_sec - start_time.tv_sec;
				Enqueue(assemblyReq, t);
				pthread_mutex_unlock(&mutexAs);
				pthread_mutex_lock(&mutexQA);
				t.reqTime = current_time.tv_sec - start_time.tv_sec;
				t.taskArr = current_time.tv_sec - start_time.tv_sec;
				Enqueue(qaReq, t);
				pthread_mutex_unlock(&mutexQA);
				// printf("Task ID %d, counter: %d, type: %d\n",t.ID,counter,t.type);
				// pthread_sleep(1);
				// printf("CONTROLLER ADDED TASK ASSEMBLY AND QA\n");
			}
		}
		else
		{
			printf("Ooops there is an error on random generation.\n");
		}

		counter++;
		pthread_sleep(1);
		// pthread_mutex_unlock(&mutexC); // RELEASE CONTROLLER
		gettimeofday(&current_time, NULL);
	}
}