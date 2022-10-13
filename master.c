#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <semaphore.h>
#include <sys/sem.h>

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int semAddress;

void signalHandler(int);

union semun {
int val;
struct semid_ds *buf; unsigned short *array;
};

int main(int argc, char *argv[])
{
    time_t rawtime;
    struct tm *timeinfo;


    FILE *logfile;

    pid_t wpid;

    //logfile strings
    char string[50] = "logfile.";
    char string1[10] = "";

    //keys
    key_t key1 = ftok(".", 15);
    key_t key2 = ftok(".", 16);
    key_t key3 = ftok(".", 17);

    //program argument variables
    int c, forks, final, status = 0;
    char *ss, *n;
    int seconds;

    //processing arguments
    while ((c = getopt(argc, argv, "t:")) !=-1)
    {
        switch(c)
        {
            case 't':
            ss = optarg;
            seconds = atoi(ss);
            if(seconds < 1)
            {
                printf("Seconds must be greater than 0");
                exit(1);
            }
            alarm((unsigned int) seconds);
            n = argv[optind];
            if(atoi(n)>20 || atoi(n)<1)
            {
                printf("The process number must be between 1 and 20. Default value will be set to 20.");
                forks = 20;

            }
            else 
            forks = atoi(n);
            break;

            case ':':
            perror("Incorrect option, try again.");
            exit(1);
            break;

            case '?':
            perror("Incorrect option, try again.");
            exit(1);
            break;
        }
    }
    //signal handlers
    signal(SIGALRM, signalHandler);
    signal(SIGINT, signalHandler);

    size_t size = (unsigned long)forks * sizeof(int);

    //semaphore allocation
    union semun arg;
    arg.val = 1;
    semAddress = semget(key1, 1, PERMS | IPC_CREAT);
    
    if(semAddress == -1)
    perror("Error semAddress");

    int sem = semctl(semAddress, 0, SETVAL, arg.val);
    if(sem == -1)
    perror("Error sem");

    int value = semctl(semAddress, 0, GETVAL);
    if(value < 0)
    perror("Error value");

    printf("Semaphore initialized.\n");

    char *args[]={"./slave", ss, n, NULL};

    //forking child processes
    for(int i = 0; i <forks; i++)
    {
        if((fork())==0)
        {  
            sprintf(args[2], "%d", i);
            execv(args[0], args);
            perror("Failed");
        }
        printf("Process #%d has been launched.\n", i+1);
        //writing to logfiles
        sprintf(string1, "%d", i+1);
        strcat(string, string1);
        logfile = fopen(string, "a");
        fputs("Process ", logfile);
        fprintf(logfile, "%d", i+1);
        time(&rawtime );
        timeinfo = localtime ( &rawtime );
        fputs(" is in the critical section at time: ", logfile);
        fprintf(logfile, "%s", asctime(timeinfo));
        fputs("\n", logfile);
        fclose(logfile);
        strcpy(string, "logfile.");
    }
    //waiting for processes to finish if signal not caught
    while ((wpid = wait(&status)) > 0);
    //memory deallocation and exit
    semctl(semAddress, 0, IPC_RMID);
    printf("Semaphore is now destroyed.\n");
    exit(0);
}

void signalHandler(int num)
{   
    printf("\nSignal received: %d", num);
    semctl(semAddress, 0, IPC_RMID);
    
    
    exit(1);
}
