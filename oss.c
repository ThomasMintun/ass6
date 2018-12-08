#include "header.h"

void testOutputs();//for testing
void sigint(int);//for ctrl-c and segfaults
void checkMsgQ();//checks message queue if 1 then read messages
void cleanup();//deletes shared memory and message queue
void createProcess();//generate forks
void runCountCheckForTermination();//after all processes run 1000 times this will terminate the program
void memManagement();
void checkPageTable();
void searchFrameTable(int pageTable, int processNumber);
void printResults(int option, int processNumber, int optionTwo);//print the results
//void initializeDirtyBits();

int main(int argc, char *argv[]) {
    int opt;
        while((opt = getopt (argc, argv, "h")) != -1)
                switch (opt){
                        case 'h':
                            printf("Program spawns user children, uses some shared memory, and message queues.\n");
                            printf("Program will write to and create file log.txt \n");
                            printf("Program can take one optional argument that limits amount of concurrent processes. \n");
                            return 0;
                            break;
                }

    //handle arguments
    if(argc > 1){
        perror("Error: Arguments must be integer greater than 0 and less than 18. Setting to default: 18");
    }

    sharedMemoryConfig();//set up shared memory 
    messageQueueConfig();//set up message queue
    signal(SIGINT, sigint);//for ctrl-c termination
    signal(SIGSEGV, sigint);//for seg faults
    initRandomForkTimes();//generate number between 100-500 for random times to fork off processes
    //initializeDirtyBits();

    while(procsRunning == 0){//main loop
        createProcess();
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
    int counter;
    for(counter = 0; counter < 18; counter++){
        printf("%d ", mainPIDHolder[counter]);
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
    int counter;
    for(counter = 0; counter < 18; counter++){
        if(mainPIDHolder[counter] == PID) {
            mainPIDHolder[counter] = 0;
        }
    }
    strcpy(message.mesg_text, "0");
}

void cleanup(){
    if ( msgctl(msgid,IPC_RMID,0) < 0 ){
        perror("msgctl");
    }
    // kill open forks
    int counter;
    for(counter = 0; counter < 18; counter++){
        if(mainPIDHolder[counter] != 0){
            signal(SIGQUIT, SIG_IGN);
            kill(mainPIDHolder[counter], SIGQUIT);
        }
    }
    //clean shared memory
    shmdt(sharedShmptr);
    shmctl(sharedShmid, IPC_RMID, NULL);
    //destroy the message queue
    msgctl(msgid, IPC_RMID, NULL);
}

void createProcess(){
    int counter;
    //fork into the pidholder postions with arr[pos] = 0;
    for(counter = 0; counter < 18; counter++){
        if(mainPIDHolder[counter] == 0){
            int positionPID = counter;
            char stashbox[10];
            sprintf(stashbox, "%d", positionPID);
            // creates process in the pidHolder at
            if ((mainPIDHolder[counter] = fork()) == 0) {
                // argv{0] is page table number
                execl("./user", "user", stashbox, NULL);//sends page table number to the stashbox
            }
            printf("\nfork made with PID: %d\n", mainPIDHolder[counter]);
            break;
        }
    }
        //nothing below this is access, until end of statement
}

void runCountCheckForTermination(){
    //check run counts
    int counter;
    for(counter = 0; counter < 18; counter++){
        if(sharedShmptr -> processCallCount[counter] >= 10)
            mainPIDHolder[counter] = 1;//writes 1 into pidHolder if that pidHolder has ran 1000 processes
    }

    int sum = 0;
    //terminate if all 1's
    for(counter = 0; counter < 18; counter++){//count up the 1s in pidHolder, if 1s in pidHolder = 18 then terminate program
        if(mainPIDHolder[counter] == 1)
            sum++;
    }

    if(sum >= 18){
        procsRunning = 1;
    }
}

void memManagement(){//increments the clock 
    int counter;
    //check page table
    
    for (counter = 0; counter < 18; counter++){
        if (mainPIDHolder[counter] == PID){
            printf("%d ", mainPIDHolder[counter]);
            if(sharedShmptr->processReadOrWrite[counter] == 0){
            cLock.milliseconds += 10;
            printResults(1, counter, 1);
            }
            else if (sharedShmptr->processReadOrWrite[counter] == 1){
                cLock.milliseconds += 51;
                printResults(2, counter, 2);
            }
        //check page table at position counter        
        break;
        }
    }
}

void checkPageTable(int processNumber){
    //find page
    int pageNumber;
    pageNumber = sharedShmptr->processAddressCalled[processNumber] / 1000;

    if(pageTable->pages[pageNumber] == 0){
        printResults(3, processNumber, pageNumber);
        searchFrameTable(pageNumber, processNumber);
    }
    //else if (pageTable[processs])
}

void searchFrameTable(int pageNumber, int processNumber){
    int counter;
    for (counter = 0; counter < 256; counter++){
        if(frameTable.frames[counter] == 0){
            frameTable.frames[counter] = pageNumber;
            printResults(4, processNumber, pageNumber);

            if (sharedShmptr->processReadOrWrite[processNumber] == 1)
                frameTable.dirtyBit[counter] = 'd';
                printResults(5, processNumber, pageNumber);
        }
    }
}

// if process completes, write data to log
void printResults(int option, int processNumber, int optionTwo){
    FILE *fp = fopen("log.txt", "a+");

    switch(option){
        case 1:
            fprintf(fp, "Process:%d requesting read of address %d at time %d \n", processNumber, sharedShmptr->processAddressCalled[processNumber], cLock.milliseconds);
            break;
        case 2:
            fprintf(fp, "Process:%d requesting write of address %d at time %d \n", processNumber, sharedShmptr->processAddressCalled[processNumber], cLock.milliseconds);
            break;
        case 3:
            fprintf(fp, "Address:%d is not in a fram resulting in a page fault. \n", sharedShmptr->processAddressCalled[processNumber]);
            break;
        case 4:
            fprintf(fp, "Address:%d \n", sharedShmptr->processAddressCalled[processNumber]);
            break;
        case 5:
            fprintf(fp, "Dirty bit of frame %d set. Adding additional time to clock. \n", optionTwo);
            break;
        case 6:
            fprintf(fp, "Dirty bit of frame %d set. Adding additional time to clock. \n", optionTwo);
            break;
    }
    fclose(fp);
}

// void initializeDirtyBits(){
//     int counter;
//     for (counter = 0; counter < 256; counter++){
//         frameTable = 4;
//     }
// }
