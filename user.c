#include "header.h"

void getUserVariables(int);
void writeMessageQ();

int main(int argc, char* argv[]) {
    //gets proc number from execl argv[1]
    char *p;
    int pageTableID = strtol(argv[1], &p, 10);

    sharedMemoryConfig();//set up shared memory
    messageQueueConfig();//set up message queues
    getUserVariables(pageTableID);
    writeMessageQ();

    shmdt(sharedShmptr);//clean shared mem
    printf("\n Process with PID: %d \n", getpid());
    exit(0);
}

void getUserVariables(int pageTableID){
    printf("\n Page table process id is: %d \n", pageTableID);
    srand(getpid());//init clock to random

    //process address space called 0-32k
    sharedShmptr -> processAddressCalled[pageTableID] = (rand() % (31999-1)) + 1;
    int readOrWrite = (rand() % 9);
    if(readOrWrite <= 5){
        //read is 0
        sharedShmptr -> processReadOrWrite[pageTableID] = 0;
    } else {
        //write is 1
        sharedShmptr -> processReadOrWrite[pageTableID] = 1;
    }
    //increment call count
    sharedShmptr -> processCallCount[pageTableID]++;
    //check proc number passed correctly
    sharedShmptr -> checkProcNum[pageTableID] = pageTableID;
}

void writeMessageQ(){
    sprintf(message.mesg_text, "%d", getpid());
    //strcpy(message.mesg_text, "A message from the msgQ");
    message.mesg_type = 1;
    //msgsnd to send message queue
    msgsnd(msgid, &message, sizeof(message), 0);
}

