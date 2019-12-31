#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>

#define OK_TAG   "[\x1b[32m OK \x1b[0m] "
#define SEM_KEY 14839
#define MEM_KEY 94029
#define SEG_SIZE 4

/* union semun { */
/*   int              val;    /\* Value for SETVAL *\/ */
/*   struct semid_ds *buf;    /\* Buffer for IPC_STAT, IPC_SET *\/ */
/*   unsigned short  *array;  /\* Array for GETALL, SETALL *\/ */
/*   struct seminfo  *__buf;  /\* Buffer for IPC_INFO */
/*                               (Linux-specific) *\/ */
/* }; */

void semaphore_init(){
    int semd;
    int v, r;

    printf("\n--- SEMAPHORE INITIALIZATION ---\n");
    semd = semget(SEM_KEY, 1, 0);
    if(semd){
        semctl(semd, 0, IPC_RMID, 0);
    }
    printf(OK_TAG "cleaned existing semaphores\n");    
    semd = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0644);
    if (semd == -1) {
        printf("error %d: %s\n", errno, strerror(errno));
        semd = semget(SEM_KEY, 1, 0);
        v = semctl(semd, 0, GETVAL, 0);
        printf("semctl returned: %d\n", v);
    }
    else {
        union semun us;
        us.val = 1;
        r = semctl(semd, 0, SETVAL, us);
        printf(OK_TAG "initialized semaphore, semctl returned: %d\n", r);
    }

}

void shmem_init(){
    int shmd;
    char *data;

    printf("--- SHARED MEMORY INITIALIZATION ---\n");
    shmd = shmget(MEM_KEY, SEG_SIZE, IPC_CREAT | 0644);
    data = shmat(shmd, 0, 0);
    printf(OK_TAG "new shared memory segment\n");
    printf(OK_TAG "current contents: [%s]\n", data);

    shmdt(data);

    
}

void semaphore_clean(){
    int semd;
    printf("\n--- SEMAPHORE CLEANING ---\n");
    semd = semget(SEM_KEY, 1, 0);
    if(semd){
        semctl(semd, 0, IPC_RMID, 0);
        printf(OK_TAG "removed semaphore\n");
    } else {
        printf(OK_TAG "no semaphore to remove\n");
    } 
}

void shmem_clean(){
    int shmd;
    char *data;

    printf("--- SHARED MEMORY CLEANING ---\n");
    shmd = shmget(MEM_KEY, SEG_SIZE, IPC_CREAT | 0644);
    shmctl(shmd, IPC_RMID, 0);
    printf(OK_TAG "cleaned memory\n");
}

int main(){
    char input[3];

    printf("\nInitialize or Clean?(i/c) ");
    fgets(input, 3, stdin);
    if(input[0] == 'i'){
        semaphore_init();
        shmem_init();
    } else {
        semaphore_clean();
        shmem_clean();
    }   
}
