#include "process_manager.h"

/*
 * Function 1: Basic Producer-Consumer Demo
 * Creates one producer child (sends 1,2,3,4,5) and one consumer child (adds them up)
 */
int run_basic_demo(void) {
    int pipe_fd[2];
    pid_t producer_pid, consumer_pid;
    int status;
    
    printf("\nParent process (PID: %d) creating children...\n", getpid());
    
    if (pipe(pipe_fd) == -1) {
       perror("pipe failed");
       return -1;
    }

    producer_pid = fork();
    if(producer_pid == 0) {
      close(pipe_fd[0]);
      producer_process(pipe_fd[1], 1);
      exit(0);
    }
    else if (producer_pid > 0) {
        printf("Created producer child (PID: %d) \n", producer_pid);
    }
    else {
        perror("fork failed");
        return -1;
    }  

    consumer_pid = fork();
    if(consumer_pid == 0) {
      close(pipe_fd[1]);
      consumer_process(pipe_fd[0], 0);
      exit(0);
    }
    else if (consumer_pid > 0) {
         printf("Created consumer child (PID: %d) \n", consumer_pid);
    }
    else {
        perror("fork failed");
        return -1;
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    
    pid_t producer_child = waitpid(producer_pid, &status, 0);
    printf("Producer child (PID: %d) exited with status %d \n", producer_child, status);

    pid_t consumer_child = waitpid(consumer_pid, &status, 0);
    printf("Consumer child (PID: %d) exited with status %d \n", consumer_child, status);
    
    return 0;
}

/*
 * Function 2: Multiple Producer-Consumer Pairs
 * Creates multiple pairs: pair 1 uses numbers 1-5, pair 2 uses 6-10, etc.
 */
int run_multiple_pairs(int num_pairs) {
    pid_t pids[10]; // Store all child PIDs
    int pid_count = 0;

    printf("\nParent creating %d producer-consumer pairs...\n", num_pairs);

    
    for(int i = 0; i < num_pairs; i++) {
       int pipe_fd[2];
       if(pipe(pipe_fd) == -1) {
          perror("pipe failed");
          return -1;
       }
       
       printf("\n=== Pair %d ===\n", i + 1);
       
       pid_t producer_pid = fork();
       if(producer_pid == 0) {
         close(pipe_fd[0]);
         producer_process(pipe_fd[1], i * 5 + 1);
         exit(0);
       }
       else if (producer_pid > 0) {
            pids[pid_count++] = producer_pid;
       }
       else {
          perror("fork failed");
          return -1;
       }
 
       pid_t consumer_pid = fork();
       if(consumer_pid == 0) {
          close(pipe_fd[1]);
          consumer_process(pipe_fd[0], i +1);
          exit(0);
       }
       else if (consumer_pid > 0) {
           pids[pid_count++] = consumer_pid;
       }
       else {
          perror("fork failed");
          return -1;
       }
       close(pipe_fd[0]);
       close(pipe_fd[1]);
    }   


    printf("\nAll pairs completed successfully!\n");

    for (int i = 0; i < pid_count; i++) {
        int status;
        pid_t child_pid = waitpid(pids[i], &status, 0);
        printf("Child (PID: %d) exited with status %d \n", child_pid, status);
    }
       
    return 0;
}

/*
 * Producer Process - Sends 5 sequential numbers starting from start_num
 */
void producer_process(int write_fd, int start_num) {
    printf("Producer (PID: %d) starting...\n", getpid());
    
    // Send 5 numbers: start_num, start_num+1, start_num+2, start_num+3, start_num+4
    for (int i = 0; i < NUM_VALUES; i++) {
        int number = start_num + i;
        
        if (write(write_fd, &number, sizeof(number)) != sizeof(number)) {
            perror("write");
            exit(1);
        }
        
        printf("Producer: Sent number %d\n", number);
        usleep(100000); // Small delay to see output clearly
    }
    
    printf("Producer: Finished sending %d numbers\n", NUM_VALUES);
    close(write_fd);
    exit(0);
}

/*
 * Consumer Process - Receives numbers and calculates sum
 */
void consumer_process(int read_fd, int pair_id) {
    int number;
    int count = 0;
    int sum = 0;
    
    printf("Consumer (PID: %d) starting...\n", getpid());
    
    // Read numbers until pipe is closed
    while (read(read_fd, &number, sizeof(number)) > 0) {
        count++;
        sum += number;
        printf("Consumer: Received %d, running sum: %d\n", number, sum);
    }
    
    printf("Consumer: Final sum: %d\n", sum);
    close(read_fd);
    exit(0);
}
