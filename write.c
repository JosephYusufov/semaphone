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
#define INPUT_MAX 256

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                              (Linux-specific) */    
};

int main(){
    int semd;
    int shmd;
    char *input = calloc(INPUT_MAX, sizeof(char));
    int *shm_input_len;
    char *previous = calloc(INPUT_MAX, sizeof(char));
    int fd, r, w, l;
    shmd = shmget(MEM_KEY, SEG_SIZE, 0);
    shm_input_len = shmat(shmd, 0, 0);

    // Getting semaphore
    semd = semget(SEM_KEY, 1, 0);
    if (semd == -1) {
        printf("error %d: semget(): %s\n", errno, strerror(errno));
	return 1;
    } else {
	printf(OK_TAG "recieved semaphore ID\n");	
    }

    // opening file to read last input using current state of shared mem
    fd = open("./story.txt", O_RDWR, 0644);
    if (fd == -1) {
        printf("error %d: open(): %s\n", errno, strerror(errno));
	return 1;
    } else {
	printf(OK_TAG "opened file\n");	
    }

    // setting the cursor to the proper position 
    /* l = lseek(fd, 0, SEEK_END); */
    /* printf("[%d]\n", l); */
    /* l = lseek(fd, l - (*shm_input_len), SEEK_END); */
    l = lseek(fd, -(*shm_input_len), SEEK_END);
    if (l == -1) {
        printf("error %d: lseek(): %s\n", errno, strerror(errno));
	return 1;
    } else {
	printf(OK_TAG "changed cursor\n");
	printf("[%d]\n", l);
    }

    // reading
    r = read(fd, previous, *shm_input_len);
    if (r == -1) {
        printf("error %d: read(): %s\n", errno, strerror(errno));
	return 1;
    } else {
	printf(OK_TAG "read last addition\n");
	printf("\tLast addition:\n");
	printf("%s\n", previous);
    }
    close(fd);

    // Configuring semaphore struct
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = -1;

    // Accepting user input
    printf("\tType text to be added to the story:\n> ");
    fgets(input, INPUT_MAX, stdin);


    // Upping the semaphore
    printf(OK_TAG "waiting for access...\n");
    semop(semd, &sb, 1);
    
    // Updating the Shared Memory segment
    *shm_input_len = strlen(input);
    printf("shm: [%d]\n", *shm_input_len);

    // Writing to the story.txt file
    /* *strchr(input, '\n') = '\0'; */
    fd = open("./story.txt", O_RDWR | O_APPEND, 0644);
    w = write(fd, input, *shm_input_len);
    if (w == -1) {
        printf("error %d: write(): %s\n", errno, strerror(errno));
	return 1;
    } else {
	printf(OK_TAG "wrote your addition\n");
    }
    
    // Downing the semaphore
    sb.sem_op = 1;
    semop(semd, &sb, 1);

    // Tying up loose ends
    shmdt(shm_input_len);        
    close(fd);
    free(previous);
    free(input);
    return 0;    
}
