#include "header.h"

void testOutputs();//for testing
void sigint(int);//for ctrl-c and segfaults
void checkMsgQ();//checks message queue if 1 then read messages
void cleanup();//deletes shared memory and message queue
void createProcess();//generate forks
void runCountCheckForTermination();//after all processes run 1000 times this will terminate the program
void memManagement();
void printResults();//print the results

int main(int argc, char *argv[]) {
    int opt;
        while((opt = getopt (argc, argv, "h")) != -1)
                switch (opt){
                        case 'h':
                                printf("Program spawns user children, uses some shared memory, and message queues.\n");
                                printf("Program will write to and create file log.txt \n");
                                printf("Program can take one optional argument that limits amount of concurrent processes. \n");
                                return 0;
                                break;}

    //handle arguments
    if(argc > 1){
        perror("Error: Arguments must be integer greater than 0 and less than 18. Setting to default: 18");
    }

    sharedMemoryConfig();//set up shared memory 
    messageQueueConfig();//set up message queue
    signal(SIGINT, sigint);//for ctrl-c termination
    signal(SIGSEGV, sigint);//for seg faults

    initRandomForkTimes();//generate number between 100-500 for random times to fork off processes

    while(procsRunning == 0){//main loop
        //fork processes if time is reached and
        //pidHolder is not all 1's

        createProcess();

        //check if any of the max run counts have been met
        //if so place a 1 in the pidHolder position
        //runCountCheckForTermination();

        //sleep(1);
        checkMsgQ();
        memManagement();
        //do the processes work
        //increment the cLock
        runCountCheckForTermination();
    }
    testOutputs();
    cleanup();

    return 0;
}

//test outputs
void testOutputs(){//comment this p
    //##### OUTPUTS FOR CHECKING #####
    int xxx;
    for(xxx = 0; xxx < 18; xxx++){
        printf("%d ", mainPIDHolder[xxx]);
    }

    printf("\n ref: %d \n",sharedShmptr -> checkProcNum[0]);
    printf("\n addr: %d \n",sharedShmptr -> processAddressCalled[0]);
    printf("\n RW: %d \n",sharedShmptr -> processReadOrWrite[0]);
    printf("\n count: %d \n",sharedShmptr -> processCallCount[0]);

    printf("\n ref: %d \n",sharedShmptr -> checkProcNum[1]);
    printf("\n addr: %d \n",sharedShmptr -> processAddressCalled[1]);
    printf("\n RW: %d \n",sharedShmptr -> processReadOrWrite[1]);
    printf("\n count: %d \n",sharedShmptr -> processCallCount[1]);

    printf("\n addr: %d \n",sharedShmptr -> processAddressCalled[2]);
    printf("\n RW: %d \n",sharedShmptr -> processReadOrWrite[2]);
    printf("\n count: %d \n",sharedShmptr -> processCallCount[2]);

    printf("\n The time is: %d \n", cLock.milliseconds);
}

//handles control c
void sigint(int a) {

    testOutputs();

    //write to log

    //cleanup
    cleanup();

    printf("^C caught\n");
    exit(0);
}

void checkMsgQ(){
    int pidPass;
    //msgrcv to receive message
    msgrcv(msgid, &message, sizeof(message), 1, IPC_NOWAIT);//receiving the message, flag is 1 and we are looking for messages with flag one
    //display the message
    if(message.mesg_text[0] != '0') {
    //pidPass = atoi(message.mesg_text);
        printf("\n Data Received is : %s \n", message.mesg_text);
    }
    char *p;//using this pointer for strtol
    PID = strtol(message.mesg_text, &p, 10);//string to long
    printf("\n PID FROM MSGQ: %d\n", PID);
    int xxx;
    for(xxx = 0; xxx < 18; xxx++){
        if(mainPIDHolder[xxx] == PID) {
            mainPIDHolder[xxx] = 0;
        }
    }
    strcpy(message.mesg_text, "0");
}

void cleanup(){
    if ( msgctl(msgid,IPC_RMID,0) < 0 ){
        perror("msgctl");
    }
    // kill open forks
    int xxx;
    for(xxx = 0; xxx < 18; xxx++){
        if(mainPIDHolder[xxx] != 0){
            signal(SIGQUIT, SIG_IGN);
            kill(mainPIDHolder[xxx], SIGQUIT);
        }
    }
    //clean shared memory
    shmdt(sharedShmptr);
    shmctl(sharedShmid, IPC_RMID, NULL);
    //destroy the message queue
    msgctl(msgid, IPC_RMID, NULL);
}

void createProcess(){
    int xxx;
    //fork into the pidholder postions with arr[pos] = 0;
    for(xxx = 0; xxx < 18; xxx++){
        if(mainPIDHolder[xxx] == 0){
            int positionPID = xxx;
            char stashbox[10];
            sprintf(stashbox, "%d", positionPID);
            // creates process in the pidHolder at
            if ((mainPIDHolder[xxx] = fork()) == 0) {
                // argv{0] is page table number
                execl("./user", "user", stashbox, NULL);//sends page table number to the stashbox
            }
            printf("\nfork made with PID: %d\n", mainPIDHolder[xxx]);
            break;
        }
    }
        //nothing below this is access, until end of statement
}

void runCountCheckForTermination(){
    //check run counts
    int xxx;
    for(xxx = 0; xxx < 18; xxx++){
        if(sharedShmptr -> processCallCount[xxx] >= 10)
            mainPIDHolder[xxx] = 1;//writes 1 into pidHolder if that pidHolder has ran 1000 processes
    }

    int sum = 0;
    //terminate if all 1's
    for(xxx = 0; xxx < 18; xxx++){//count up the 1s in pidHolder, if 1s in pidHolder = 18 then terminate program
        if(mainPIDHolder[xxx] == 1)
            sum++;
    }

    if(sum >= 18){
        procsRunning = 1;
    }
}

void memManagement(){//increments the clock 
    int xxx;
    for (xxx = 0; xxx < 18; xxx++){
        if (mainPIDHolder[xxx] == PID){
            xxx = PID;//xxx is powerful and will be used
            break;
        }
    }

    if(sharedShmptr->processReadOrWrite[xxx] == 0){
        cLock.milliseconds += 10;
        printResults();
    }
    else if (sharedShmptr->processReadOrWrite[xxx] == 1){
        cLock.milliseconds += 50;
        printResults();
    }
}

// if process completes, write data to log
void printResults(){
    FILE *fp = fopen("log.txt", "a+");
    fprintf(fp, "Time is: %d\n", cLock.milliseconds);
    fclose(fp);
}
