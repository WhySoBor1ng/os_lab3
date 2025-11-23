#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_NAME "shared_memory"

#define SEM_C1C2 "sem_c1c2"
#define SEM_C2M  "sem_c2m"

#define SHM_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 1)
        return 1;

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    char *shm_ptr = mmap(NULL, SHM_SIZE,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm_fd, 0);

    sem_t *sem_c1c2 = sem_open(SEM_C1C2, 0);
    sem_t *sem_c2m  = sem_open(SEM_C2M, 0);

    while (1)
    {
        sem_wait(sem_c1c2);

        if (shm_ptr[0] == 'q')
        {
            sem_post(sem_c2m);
            break;
        }

        char result[SHM_SIZE];
        int i = 0, j = 0;
        int space = 0;

        while (shm_ptr[i])
        {
            if (shm_ptr[i] == ' ')
            {
                if (!space)
                    result[j++] = ' ';
                space = 1;
            }
            else
            {
                result[j++] = shm_ptr[i];
                space = 0;
            }
            i++;
        }
        result[j] = '\0';

        strncpy(shm_ptr, result, SHM_SIZE - 1);
        shm_ptr[SHM_SIZE - 1] = '\0';

        sem_post(sem_c2m);
    }

    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_c1c2);
    sem_close(sem_c2m);

    return 0;
}
