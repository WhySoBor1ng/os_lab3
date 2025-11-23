#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_NAME "shared_memory"

#define SEM_M2C1 "sem_m2c1"
#define SEM_C1C2 "sem_c1c2"

#define SHM_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 1)
        return 1;

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    char *shm_ptr = mmap(NULL, SHM_SIZE,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm_fd, 0);

    sem_t *sem_m2c1 = sem_open(SEM_M2C1, 0);
    sem_t *sem_c1c2 = sem_open(SEM_C1C2, 0);

    while (1)
    {
        sem_wait(sem_m2c1);

        if (shm_ptr[0] == 'q')
        {
            sem_post(sem_c1c2);
            break;
        }

        for (int i = 0; shm_ptr[i]; ++i)
        {
            shm_ptr[i] = toupper((unsigned char)shm_ptr[i]);
        }

        sem_post(sem_c1c2);
    }

    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_m2c1);
    sem_close(sem_c1c2);

    return 0;
}
