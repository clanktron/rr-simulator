#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
    u32 pid;
    u32 arrival_time;
    u32 burst_time;

    TAILQ_ENTRY(process) pointers;

    /* Additional fields here */
    u32 remaining_time;
    u32 end_time;
    u32 start_exec_time;
    u32 waiting_time;
    u32 response_time;
    /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end) {

    u32 current = 0;
    bool started = false;
    while (*data != data_end)
    {
        char c = **data;

        if (c < 0x30 || c > 0x39)
        {
            if (started)
            {
                return current;
            }
        }
        else
        {
            if (!started)
            {
                current = (c - 0x30);
                started = true;
            }
            else
            {
                current *= 10;
                current += (c - 0x30);
            }
        }

        ++(*data);
    }

    printf("Reached end of file while looking for another integer\n");
    exit(EINVAL);

}

u32 next_int_from_c_str(const char *data) {

    char c;
    u32 i = 0;
    u32 current = 0;
    bool started = false;
    while ((c = data[i++]))
    {
        if (c < 0x30 || c > 0x39)
        {
            exit(EINVAL);
        }
        if (!started)
        {
            current = (c - 0x30);
            started = true;
        }
        else
        {
            current *= 10;
            current += (c - 0x30);
        }
    }
    return current;

}

void init_processes(const char *path, struct process **process_data, u32 *process_size) {

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        int err = errno;
        perror("open");
        exit(err);
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        int err = errno;
        perror("stat");
        exit(err);
    }

    u32 size = st.st_size;
    const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data_start == MAP_FAILED)
    {
        int err = errno;
        perror("mmap");
        exit(err);
    }

    const char *data_end = data_start + size;
    const char *data = data_start;

    *process_size = next_int(&data, data_end);

    *process_data = calloc(sizeof(struct process), *process_size);
    if (*process_data == NULL)
    {
        int err = errno;
        perror("calloc");
        exit(err);
    }

    for (u32 i = 0; i < *process_size; ++i)
    {
        (*process_data)[i].pid = next_int(&data, data_end);
        (*process_data)[i].arrival_time = next_int(&data, data_end);
        (*process_data)[i].burst_time = next_int(&data, data_end);
    }

    munmap((void *)data, size);
    close(fd);

}

int main(int argc, char *argv[]) {

    if (argc != 3)
    {
        return EINVAL;
    }
    struct process *data;
    u32 size;
    init_processes(argv[1], &data, &size);

    u32 quantum_length = next_int_from_c_str(argv[2]);

    struct process_list list;
    TAILQ_INIT(&list);

    u32 total_waiting_time = 0;
    u32 total_response_time = 0;

    /* Your code here */ 
    
    // Init remaining_time to burst/run time
    for (u32 i = 0; i < size; i++)
    {
        struct process *current_process = &data[i];
        current_process->remaining_time = current_process->burst_time;
//         fprintf(stderr, "Process %d arrives at %d and has to run for %d units of time.\n", current_process->pid, current_process->arrival_time, current_process->burst_time);
    }

    // Simulate Round Robin
    u32 time = 0;
    u32 current_quantum = 0;
    u32 finished_processes = 0;
    u32 context_switches = 0;
    struct process *current_process = NULL;
    bool finished = false;
    while ( !finished ) {
        // check if there are any new arrivals to add to the queue
        for (u32 i = 0; i < size; i++)
        {
            struct process *new_process = &data[i];
            if (new_process->arrival_time == time) {
                TAILQ_INSERT_TAIL(&list, new_process, pointers);
//                 fprintf(stderr, "Adding process %d to the queue...\n", new_process->pid);
            }
        }
        
        // if current process is finished remove it from the queue
        if (current_process != NULL) {
            if (current_process->remaining_time == 0) {
                current_process->end_time = time;
                current_process->waiting_time = current_process->end_time - current_process->arrival_time - current_process->burst_time;
//                 fprintf(stderr, "Terminating process %d - quantum lasted %d, waiting time was %d, and response time was %d\n", current_process->pid, current_quantum, current_process->waiting_time, current_process->response_time);
                TAILQ_REMOVE(&list, current_process, pointers);
                current_process = NULL;
                finished_processes += 1;
                if (finished_processes == size) {
                    finished = true;
                }
            }

            // if the quantum is over move the current_process to the back of the queue
            if (current_quantum == quantum_length && current_process != NULL) {
//                 fprintf(stderr, "Pausing process %d - quantum lasted %d\n", current_process->pid, current_quantum);
                TAILQ_REMOVE(&list, current_process, pointers);
                TAILQ_INSERT_TAIL(&list, current_process, pointers);
                current_process = NULL;
            }
        }

        // start a new process if the queue isn't empty and there is no current process 
        if (!TAILQ_EMPTY(&list) && current_process == NULL) {
            current_process = TAILQ_FIRST(&list);
            current_process->start_exec_time = time;
//             fprintf(stderr, "Starting process %d", current_process->pid);
            if (current_process->remaining_time == current_process->burst_time) {
                current_process->response_time = current_process->start_exec_time - current_process->arrival_time;
//                 fprintf(stderr,  " - response time was %d", current_process->response_time);
            }
//             fprintf(stderr,"\n");
            current_quantum = 0;
            context_switches++;
        }

        if (current_process != NULL) {
            current_process->remaining_time -= 1;
        }

        time++;
        current_quantum++;
    }

    // Calculate total waiting_time and response_time
    for (u32 i = 0; i < size; i++)
    {
        struct process *current_process = &data[i];
        total_waiting_time += current_process->waiting_time;
        total_response_time += current_process->response_time;
    }
    /* End of "Your code here" */

//     fprintf(stderr, "Total time: %d\n", time);
    printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
    printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

    free(data);
    return 0;

}
