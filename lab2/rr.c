#include <fcntl.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <unistd.h>

/* A process table entry.  */
struct process
{
  long pid;
  long arrival_time;
  long burst_time;

  TAILQ_ENTRY(process)
  pointers;

  /* Additional fields here */
  long remaining_time;
  long start_exec_time;
  long waiting_time;
  long response_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

/* Skip past initial nondigits in *DATA, then scan an unsigned decimal
   integer and return its value.  Do not scan past DATA_END.  Return
   the integer’s value.  Report an error and exit if no integer is
   found, or if the integer overflows.  */
static long next_int(char const **data, char const *data_end)
{
  long current = 0;
  bool int_start = false;
  char const *d;

  for (d = *data; d < data_end; d++)
  {
    char c = *d;
    if ('0' <= c && c <= '9')
    {
      int_start = true;
      if (ckd_mul(&current, current, 10) || ckd_add(&current, current, c - '0'))
      {
        fprintf(stderr, "integer overflow\n");
        exit(1);
      }
    }
    else if (int_start)
      break;
  }

  if (!int_start)
  {
    fprintf(stderr, "missing integer\n");
    exit(1);
  }

  *data = d;
  return current;
}

/* Return the first unsigned decimal integer scanned from DATA.
   Report an error and exit if no integer is found, or if it overflows.  */
static long next_int_from_c_str(char const *data)
{
  return next_int(&data, strchr(data, 0));
}

/* A vector of processes of length NPROCESSES; the vector consists of
   PROCESS[0], ..., PROCESS[NPROCESSES - 1].  */
struct process_set
{
  long nprocesses;
  struct process *process;
};

/* Return a vector of processes scanned from the file named FILENAME.
   Report an error and exit on failure.  */
static struct process_set init_processes(char const *filename)
{
  int fd = open(filename, O_RDONLY);
  if (fd < 0)
  {
    perror("open");
    exit(1);
  }

  struct stat st;
  if (fstat(fd, &st) < 0)
  {
    perror("stat");
    exit(1);
  }

  size_t size;
  if (ckd_add(&size, st.st_size, 0))
  {
    fprintf(stderr, "%s: file size out of range\n", filename);
    exit(1);
  }

  char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    perror("mmap");
    exit(1);
  }

  char const *data_end = data_start + size;
  char const *data = data_start;

  long nprocesses = next_int(&data, data_end);
  if (nprocesses <= 0)
  {
    fprintf(stderr, "no processes\n");
    exit(1);
  }

  struct process *process = calloc(sizeof *process, nprocesses);
  if (!process)
  {
    perror("calloc");
    exit(1);
  }

  for (long i = 0; i < nprocesses; i++)
  {
    process[i].pid = next_int(&data, data_end);
    process[i].arrival_time = next_int(&data, data_end);
    process[i].burst_time = next_int(&data, data_end);
    if (process[i].burst_time == 0)
    {
      fprintf(stderr, "process %ld has zero burst time\n",
              process[i].pid);
      exit(1);
    }
  }

  if (munmap(data_start, size) < 0)
  {
    perror("munmap");
    exit(1);
  }
  if (close(fd) < 0)
  {
    perror("close");
    exit(1);
  }
  return (struct process_set){nprocesses, process};
}

//comparator function for sorting an array
int compare (const void * elem1, const void * elem2) 
{
    long f = *((long*)elem1);
    long s = *((long*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

// main program
int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "%s: usage: %s file quantum\n", argv[0], argv[0]);
    return 1;
  }

  struct process_set ps = init_processes(argv[1]);
  long quantum_length = (strcmp(argv[2], "median") == 0 ? -1
                                                        : next_int_from_c_str(argv[2]));
  if (quantum_length == 0)
  {
    fprintf(stderr, "%s: zero quantum length\n", argv[0]);
    return 1;
  }

  struct process_list list;
  TAILQ_INIT(&list);

  long total_wait_time = 0;
  long total_response_time = 0;

  /* Your code here */
  int proc_count = 0; //# of processes run
  bool dynamic_quantum = quantum_length == -1; //returns true if quantum is dynamically determined using median
  long* runtime_list = (long*)malloc(10000 * sizeof(long)); //array to keep track of process runtimes

  struct process *first_proc = &ps.process[0];
  int time = first_proc->arrival_time;
  int arrival_index = 1;
  // insert first process into the queue
  TAILQ_INSERT_TAIL(&list, &ps.process[0], pointers);
  // while queue isn't empty
  while (!TAILQ_EMPTY(&list))
  {
    if (dynamic_quantum) {
      if (quantum_length <= 0){
        quantum_length = 1;
      }
      else {
        //sort the runtime array and set quantum to its median
        qsort(runtime_list, proc_count, sizeof(long), compare);
        // for (int i = 0; i < proc_count; i++){
        //   printf("%ld", runtime_list[i]);
        // }
        quantum_length = runtime_list[proc_count/2];
        // printf("quantum length is: %ld\n", quantum_length);
      }
    }
    struct process *curr_process = TAILQ_FIRST(&list);
    printf("process %ld executes at time %d\n", curr_process->pid, time);

    // first run since arrival
    if (curr_process->start_exec_time == 0)
    {
      curr_process->start_exec_time = time;
      curr_process->remaining_time = curr_process->burst_time;
      long res = time - curr_process->arrival_time;
      curr_process->response_time = res;
      total_response_time += res;
      // printf("HERE res time: %ld \n", total_response_time);
    }

    // runtime of current process = min(remaining time, quantum length)
    int runtime = curr_process->remaining_time > quantum_length ? quantum_length : curr_process->remaining_time;
    int next_time = time + runtime + 1;

    // append newly arrived processes to queue
    while (arrival_index < ps.nprocesses && ps.process[arrival_index].arrival_time <= next_time)
    {
      TAILQ_INSERT_TAIL(&list, &ps.process[arrival_index], pointers);
      printf("process %ld arrives at time %d\n", (&ps.process[arrival_index])->pid, time);
      arrival_index++;
    }

    // run the current process
    curr_process->remaining_time -= runtime;
    proc_count++;
    runtime_list[proc_count] = curr_process->burst_time - curr_process->remaining_time;
    // printf("process number %d is: %ld\n", proc_count, runtime_list[proc_count]);
    time += runtime + 1; // add context switch + quantum time

    // remove process from queue
    TAILQ_REMOVE(&list, curr_process, pointers);

    // add current process to queue if it still has burst time left
    if (curr_process->remaining_time > 0)
    {
      TAILQ_INSERT_TAIL(&list, curr_process, pointers);
    }
    // otherwise, get the total wait time
    else
    {
      // printf("process %ld wait time: %ld \n", curr_process->pid, time - curr_process->arrival_time - curr_process->burst_time);
      total_wait_time += time - curr_process->arrival_time - curr_process->burst_time;
    }
  }

  // print out processes for reference
  // for (long i = 0; i < ps.nprocesses; i++)
  // {
  //   printf("pid: %ld ", ps.process[i].pid);
  //   printf("arrival: %ld ", ps.process[i].arrival_time);
  //   printf("burst: %ld ", ps.process[i].burst_time);
  //   printf("\n");
  // }

  /* End of "Your code here" */

  printf("Average wait time: %.2f\n",
         total_wait_time / (double)ps.nprocesses);
  printf("Average response time: %.2f\n",
         total_response_time / (double)ps.nprocesses);

  if (fflush(stdout) < 0 || ferror(stdout))
  {
    perror("stdout");
    return 1;
  }

  free(ps.process);
  return 0;
}
