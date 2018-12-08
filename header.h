#ifndef ASS6_HEADER_H
#define ASS6_HEADER_H

#include <stdio.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/wait.h>
//#include <sys/time.h>
//#include <sys/stat.h>

#define MAX_PROCS 18
#define SHMKEY 696969

// #####
// ##### STRUCTURES #####
// #####

// struct for time
typedef struct {
    unsigned int seconds;
    unsigned int milliseconds;
} systemClock_t;

//frame table
typedef struct {
    int pageTablePages[256];
    int PIDS[256];
    int referenceFlag[156];
    int dirtyBit[156];
} frameTable_t;

//page table
typedef struct {
    int frameTableFrames[32];
} pageTable_t;

//for shared memory
typedef struct {
    int sharedPIDHolder[18];         // for checking program termination
    int checkProcNum[18];
    int processAddressCalled[18];
    int processReadOrWrite[18];
    int processCallCount[18];
} shared_t;

struct mesg_buffer {//message queue
    long mesg_type; //will be 1 if there is a message is in the message queue will read the messages FIFO like a line
    char mesg_text[100]; //txt we are sending in which is RPID
} message;

//vars...
int prMax = 18; //max procs allowed
int PID;
frameTable_t frameTable;//define frame table
pageTable_t pageTable[MAX_PROCS];//define array of page tables
systemClock_t cLock;
int sharedShmid;//shmem - holds the shared memory segment id
shared_t *sharedShmptr;//shmem - points to the data structure
int msgid;//for message queue
key_t key;//for message queue

int proccessesRunning = 0;//variable that terminates program
int mainPIDHolder[18] = {};//the main pid holder which keeps track of processes
int numForksMade = 0;
int randTimeToFork[18];//function will fill this array with random numbers between one and five hundred which is the random time that they will be forked off
int procsRunning = 0;

void sharedMemoryConfig() {
    //shared mem for sysClock
    sharedShmid = shmget(SHMKEY, sizeof(systemClock_t), IPC_CREAT|0777);
    if(sharedShmid < 0)
    {
        perror("sysClock shmget error in master \n");
        exit(errno);
    }
    sharedShmptr = shmat(sharedShmid, NULL, 0);
    if(sharedShmptr < 0){
        perror("sysClock shmat error in oss\n");
        exit(errno);
    }
}

void messageQueueConfig(){
    //ftok to generate unique key
    key = ftok("progfile", 65);
    //msgget creates a message queue
    //and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);
}

void initRandomForkTimes(){
    // init clock to random
    time_t t;
    srand((unsigned) time(&t));

    int ii;
    for(ii = 0; ii < 18; ii++){
        randTimeToFork[ii] = (rand() % (500 - 100)) + 100; //between 100 and 500
    }
}

#endif