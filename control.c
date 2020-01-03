#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define OK_TAG   "[\x1b[32m OK \x1b[0m] "
#define SEM_KEY 14839
#define MEM_KEY 94029
#define SEG_SIZE 4
#define STORY_MAX 2048

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                              (Linux-specific) */    
};

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
    int *data;

    printf("--- SHARED MEMORY INITIALIZATION ---\n");
    shmd = shmget(MEM_KEY, SEG_SIZE, IPC_CREAT | 0644);
    data = shmat(shmd, 0, 0);
    *data = 0;
    printf(OK_TAG "new shared memory segment\n");
    printf(OK_TAG "current contents: [%ls]\n", data);

    shmdt(data);    
}

void file_init(){
    char *buf;
    int fd;
    int r;
    fd = open("./story.txt", O_TRUNC | O_WRONLY, 0644);   
    if (fd == -1){
	printf("error: %s\n", strerror(errno));
    } else {
	printf(OK_TAG "cleaned story file\n");
	close(fd);
    }
}

void file_clean(){
    char buf[STORY_MAX];
    int fd;
    int r;
    printf("\n--- FILE CLEANING ---\n");    
    fd = open("./story.txt", O_RDWR, 0644);
    if (fd == -1){
	printf("error: %s\n", strerror(errno));
    } else {
	printf(OK_TAG "opened story file, fd: [%d]\n", fd);
    }
    
    r = read(fd, buf, STORY_MAX);
    if (r == -1){
	printf("error: %s\n", strerror(errno));
    } else {
	printf(OK_TAG "read from story file\n");
	printf("\tStory:\n");
	printf("%s\n", buf);
    }
    close(fd);
    file_init();        
}

void semaphore_clean(){
    int semd;
    printf("\n--- SEMAPHORE CLEANING ---\n");
    semd = semget(SEM_KEY, 1, 0);
    if(semd >= 0){
        semctl(semd, 0, IPC_RMID, 0);
        printf(OK_TAG "removed semaphore\n");
    } else {
        printf(OK_TAG "no semaphore to remove\n");
	printf("semd: [%d]\n", semd);
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

int main(int argc, int *argv[]){
    char **cli_args = argv;
    if(strcmp(cli_args[1], "-c") == 0){
        semaphore_init();
        shmem_init();
	file_init();
    } else if (strcmp(cli_args[1], "-r") == 0) {	
        semaphore_clean();
        shmem_clean();
	file_clean();
    } else {
	char buf[STORY_MAX];
	int fd;
	int r;
	fd = open("./story.txt", O_RDONLY, 0644);
	if (fd == -1){
	    printf("error: open(): %s\n", strerror(errno));
	} else {
	    printf(OK_TAG "opened story file, fd: [%d]\n", fd);
	}
	
	r = read(fd, buf, STORY_MAX);
	if (r == -1){
	    printf("error: read(): %s\n", strerror(errno));
	} else {
	    printf(OK_TAG "read from story file\n");
	    printf("\tStory:\n");
	    printf("%s\n", buf);
	}
	close(fd);	
    }
    return 0;
}
