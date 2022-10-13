#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

void setsembuf(struct sembuf *s, int num, int op, int flg);
int r_semop(int semid, struct sembuf *sops, int nsops);

int semAddress;

int main(int argc, char* argv[])
{   
    //variables
    struct sembuf wait[0];
    struct sembuf signal[0];
    FILE *cstest;

    int error;
    const int i = atoi(argv[2]);

    key_t key1 = ftok(".", 15);
    key_t key2 = ftok(".", 16);
    key_t key3 = ftok(".", 17);

    //semaphore initialization
    semAddress = semget(key1, 1, PERMS);
    setsembuf(wait, 0, -1, 0);
    setsembuf(signal, 0, 1, 0);

    //critical section loop
    for(int j = 0; j<3; j++)
    {
        if (((error = r_semop(semAddress, wait, 1)) == -1) && (i > 1)) 
        {
            perror("Child failed to lock semid");
            return 1; 
        } 
        
            //writing to critical section
        else if (!error) {
            sleep(1);
            cstest = fopen("cstest", "a");
            fputs("Process ", cstest);
            fprintf(cstest, "%d", i+1);
            fputs(" is writing to cstest\n",cstest);
            fclose(cstest);
        }
            // Exit section
            if ((error = r_semop(semAddress, signal, 1)) == -1)
            perror("Failed to unlock semid");
    }
    perror("Last error");
    exit(0);

}

void setsembuf(struct sembuf *s, int num, int op, int flg) 
{ 
    s->sem_num = (short)num;
    s->sem_op = (short)op;
    s->sem_flg = (short)flg;
    return; 
}

int r_semop(int semid, struct sembuf *sops, int nsops) 
{ 
    while (semop(semid, sops, nsops) == -1)
      if (errno != EINTR)
         return -1;
    return 0; 
}