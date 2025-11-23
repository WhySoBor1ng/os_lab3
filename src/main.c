#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_NAME "shared_memory"

#define SEM_M2C1 "sem_m2c1"
#define SEM_C1C2 "sem_c1c2"
#define SEM_C2M  "sem_c2m"

#define SHM_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        return 1;
    }

    shm_unlink(SHM_NAME);
    sem_unlink(SEM_M2C1);
    sem_unlink(SEM_C1C2);
    sem_unlink(SEM_C2M);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        const char msg[] = "Error: shm_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1)
    {
        const char msg[] = "Error: ftruncate failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char *shm_ptr = mmap(NULL, SHM_SIZE,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        const char msg[] = "Error: mmap failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_m2c1 = sem_open(SEM_M2C1, O_CREAT, 0666, 0);
    sem_t *sem_c1c2 = sem_open(SEM_C1C2, O_CREAT, 0666, 0);
    sem_t *sem_c2m  = sem_open(SEM_C2M,  O_CREAT, 0666, 0);

    if (sem_m2c1 == SEM_FAILED ||
        sem_c1c2 == SEM_FAILED ||
        sem_c2m  == SEM_FAILED)
    {
        const char msg[] = "Error: sem_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    switch (pid)
    {
        case -1:
        {
            const char msg[] = "Error: fork child1 failed\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
        case 0:
        {
            execl("./child1", "child1", NULL);
            const char msg[] = "Error: execl child1 failed\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            _exit(EXIT_FAILURE);
        }
        default:
            break;
    }

    pid = fork();
    switch (pid)
    {
        case -1:
        {
            const char msg[] = "Error: fork child2 failed\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
        case 0:
        {
            execl("./child2", "child2", NULL);
            const char msg[] = "Error: execl child2 failed\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            _exit(EXIT_FAILURE);
        }
        default:
            break;
    }

    const char msg[] = "Input text:\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    char buffer[256];
    ssize_t bytes_read;

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';

        strncpy(shm_ptr, buffer, SHM_SIZE - 1);
        shm_ptr[SHM_SIZE - 1] = '\0';

        sem_post(sem_m2c1);
        sem_wait(sem_c2m);

        if (buffer[0] == 'q')
        {
            break;
        }

        write(STDOUT_FILENO, "Processed string: ", 18);
        write(STDOUT_FILENO, shm_ptr, strlen(shm_ptr));
    }

    wait(NULL);
    wait(NULL);

    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);

    sem_close(sem_m2c1);
    sem_close(sem_c1c2);
    sem_close(sem_c2m);

    shm_unlink(SHM_NAME);
    sem_unlink(SEM_M2C1);
    sem_unlink(SEM_C1C2);
    sem_unlink(SEM_C2M);

    return 0;
}
