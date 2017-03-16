/******************
Instruction To run Code:
run ./shm_sem on bash
*******************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define SHM_SIZE 1024  /* make it a 1K shared memory segment */
#define MAX_RETRIES 10 /*Max Retries*/

union semun {
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
    ushort *array;          /* array for GETALL & SETALL */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
    void *__pad;
};
int initsem(key_t key, int nsems)  /* key from ftok() */
{
    int i;
    union semun arg;
    struct semid_ds buf;
    struct sembuf sb;
    int semid;

    /*Return Semphore Set ID if already exists 
    or Creates a new sempahore set and returns the ID*/
    semid = semget(key, nsems, IPC_CREAT | 0666);
    if (semid >= 0) 
    { /* we got it first */
        sb.sem_op = 1; sb.sem_flg = 0;
        arg.val = 1;
        for(sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) 
        { 
            /* do a semop() to "free" the semaphores. */
            /* this sets the sem_otime field, as needed below. */
            if (semop(semid, &sb, 1) == -1) 
            {
                int e = errno;
                semctl(semid, 0, IPC_RMID); /* clean up */
                errno = e;
                return -1; /* error, check errno */
            }
        }

    } 
    else {
        return semid; /* error, check errno */
    }
    return semid;
}
int main(int argc, char *argv[])
{
    key_t key;
    int shmid,i;
    pid_t pid;
    char *data,msg[SHM_SIZE];
    int mode;
    int semid;
    struct sembuf sb;
    union semun arg;
    sb.sem_num = 0;  /*Operations on Semaphore at 0th index*/
    sb.sem_op = -1;  /* set to allocate resource */
    sb.sem_flg = SEM_UNDO; /*Undo the changes after termination of process*/
    /* make the key: */
    if ((key = ftok("shm_sem.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }
    /*Initializing the new Sempahore Set*/
    if ((semid = initsem(key, 1)) == -1) {
        perror("initsem");
        exit(1);
    }
    /*Setting Initial Value of Sempahore to 1*/
    arg.val=1;
    if(semctl(semid,0,SETVAL,arg)==-1){
        perror("semctl");
    }
    /* connect to (and possibly create) the segment: */
    if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }
    /* attach to the segment to get a pointer to it: */
    data = shmat(shmid, (void *)0, 0);
    if (data == (char *)(-1)) {
        perror("shmat");
        exit(1);
    }
    /*Erasing Data already present in shared memory*/
    strcpy(data,"");
    pid=fork();//Forking
    /*Child Process*/
    if(pid == 0)
    {
        /*Locking The Semaphore*/
        if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
        }
        /*Critical Section(Reading from Shared Memory)*/
        printf("Child reading from segment: %s\n",data);
        /*Unlocking The Semaphore*/
        sb.sem_op = 1; 
        if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
        }
    }
    /*Parent Process*/
    else if(pid != 0)
    {
        /*Locking The Semaphore*/
        if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
        }
        /*Critical Section(Writing message to Shared Memory)*/
        printf("Enter Your Message\n");
        scanf("%[^\n]s",msg);
        printf("Parent writing to segment: %s\n",msg);
        strncat(data, msg, SHM_SIZE);
        /*Unlocking The Semaphore*/
        sb.sem_op = 1; 
        if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
        }
    }
    return 0;
}
