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
  bool already_start;
  long remaining_time;
  long waiting_time;
  long response_time;
  long run_time;
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

//comparator function for sorting the runtime int array
int compare_ints (const void * elem1, const void * elem2) 
{
    long a = *((long*)elem1);
    long b = *((long*)elem2);
    if (a > b) return 1;
    if (a < b) return -1;
    return 0;
}

//comparator function for sorting the process list array (stable)
int compare_arrivals (const void *elem1, const void *elem2)
{
  struct process a = *((struct process*) elem1);
  struct process b = *((struct process*) elem2);
  if (a.arrival_time > b.arrival_time) return 1;
  if (a.arrival_time < b.arrival_time) return -1;
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
  int proc_count = 0; //# of processes already run
  int proc_running = 0;
  bool dynamic_quantum = quantum_length == -1; //returns true if quantum is determined using median
  qsort(ps.process, ps.nprocesses, sizeof(struct process), compare_arrivals); //stable sort processes based on arrival
  // for (int i = 0; i < ps.nprocesses; i++){
  //   printf("%ld, ", (&ps.process[i])->pid);
  // }
  // printf("\n");

  struct process *first_proc = &ps.process[0];
  int time = first_proc->arrival_time;
  int arrival_index = 1;
  // insert first process into the queue
  TAILQ_INSERT_TAIL(&list, &ps.process[0], pointers);
  
  // while queue isn't empty
  while (!TAILQ_EMPTY(&list)){
    struct process *curr_process = TAILQ_FIRST(&list);

    // first time a process runs
    if (!curr_process->already_start){      
      curr_process->already_start = true;
      proc_count++;
      curr_process->remaining_time = curr_process->burst_time;
      long res = time - curr_process->arrival_time;
      curr_process->response_time = res;
      total_response_time += res;
    }

    // printf("num running: %d\n", proc_running);

    // printf("===== clock time: %ld =====\n", time);
    // struct process *p;
    // TAILQ_FOREACH(p, &list, pointers) {
    //   printf("pid: %ld\n", p->pid);
    // }


    //get the quantum median
    if (dynamic_quantum) {
      if (quantum_length <= 0){
        quantum_length = 1;
      }
      else{
        struct process *proc_i;
        //calculate current # of processes running
        proc_running = 0;
        TAILQ_FOREACH(proc_i, &list, pointers) {
          proc_running++;
        }
        long temp_list[proc_running];
        int index = 0;
        TAILQ_FOREACH(proc_i, &list, pointers) {
          // printf("pid %ld has runtime %ld \n", proc_i->pid, proc_i->run_time);
          temp_list[index] = proc_i->run_time;
          index++;
        }

        qsort(temp_list, proc_running, sizeof(long), compare_ints);
        int median = temp_list[proc_running / 2];
        if (proc_running != 0 && proc_running % 2 == 0){
          int first = temp_list[proc_running / 2 - 1];
          int middle = (first + median) / 2;
          //round to the nearest integer, breaking ties by rounding to even
          median = (int) middle / 1;
          if (middle - median > 0.5 || (middle - median == 0.5 && median % 2 == 1)){
            median++;
          }
        }

        //if resulting median is 0, set to 1
        if (median <= 0){
          quantum_length = 1;
        } else {
          quantum_length = median;
        }

        // for (int i = 0; i < proc_running; i++){
        //   printf("%ld, ", temp_list[i]);
        // }
        // printf("\nmedian is: %ld\n", quantum_length);
      }
    }
  
  // printf("process %ld executes at time %d\n", curr_process->pid, time);        
  // printf("\n");

    // runtime of current process = min(remaining time, quantum length)
    int runtime = curr_process->remaining_time > quantum_length ? quantum_length : curr_process->remaining_time;
    int next_time = time + runtime;

    // append newly arrived processes to queue
    while (arrival_index < ps.nprocesses && ps.process[arrival_index].arrival_time < next_time){
      TAILQ_INSERT_TAIL(&list, &ps.process[arrival_index], pointers);
      // printf("process %ld arrives at time %d\n", (&ps.process[arrival_index])->pid, time);
      arrival_index++;
    }

    // run the current process
    // printf("remaining: %ld\n", curr_process->remaining_time);
    curr_process->remaining_time -= runtime;
    curr_process->run_time += runtime;
    // printf("runtime: %d\n", runtime);
    // printf("remaining: %ld\n", curr_process->remaining_time);
    time = next_time; // set time = before next context switch

    // remove process from queue
    TAILQ_REMOVE(&list, curr_process, pointers);

    // add current process to queue if it still has burst time left
    if (curr_process->remaining_time > 0){
      TAILQ_INSERT_TAIL(&list, curr_process, pointers);
    }
    // otherwise, the process is done running and we get its total wait time (minus next context switch). remove from runtime list.
    else{
      curr_process->remaining_time = 0;
      // printf("# running: %d ", proc_running);
      total_wait_time += time - curr_process->arrival_time - curr_process->burst_time;
    }

    // for processes that arrive late.
    if (TAILQ_EMPTY(&list) && proc_count < ps.nprocesses){
      struct process *next_proc = &ps.process[arrival_index];
      TAILQ_INSERT_TAIL(&list, next_proc, pointers);
      arrival_index++;
      time = next_proc->arrival_time;
    }

    //add context switch if we run a diff process next
    if (TAILQ_FIRST(&list) != curr_process){
      time += 1;
      // printf("CONTEXT SWITCH %d\n", time);
    }
  }
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
