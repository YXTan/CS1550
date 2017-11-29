//Yongxin Tan Yot13
//CS1550 Project2

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define __NR_cs1550_down 325
#define __NR_cs1550_up 326

struct cs1550_sem {
  int value;
  //use a linked queue to implement the process queue
  struct node* head;
  struct node* tail;
};

//syscall down and up (from assignment sheet)
void up(struct cs1550_sem* sem){
  syscall(__NR_cs1550_up, sem);
}

void down(struct cs1550_sem* sem){
  syscall(__NR_cs1550_down, sem);
}

int main(int argc, char* argv[]) {

  int producer = 0;
  int consumer = 0;
  int size = 0; //buffer size

  if (argc != 4) { //check if user enters legal arguments
    printf("Illegal number of arguments! Usage: prodcon <#consumer> <#producer> <size>\n");
    return 1;
  } else { //parse the command line arguments
    //convert a sting to an int
    consumer = strtol(argv[1], NULL, 10);
    if (consumer == 0) printf("ERROR, consumer number cannot be zero!\n");
    producer = strtol(argv[2], NULL, 10); 
    if (producer == 0) printf("ERROR, producer number cannot be zero!\n");
    size = strtol(argv[3], NULL, 10);
    if (size == 0) printf("ERROR, size number cannot be zero!\n");
  }

  //allocate space for semaphores to store their information using mmap
  struct cs1550_sem* sem_mem = (struct cs1550_sem*)mmap(NULL, sizeof(struct cs1550_sem*)*3, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

  //allocate space for shared memory based on the size variable
  int* shared_mem = (int*)mmap(NULL, (size+2)*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

  //two veriables to record current producer/consumer
  int current_producer = 0;
  int current_consumer = 0;

  //semaphore empty(n), full(0), mutex(1), initialize them to these values
  
  //first semaphore to record empty spaces
  struct cs1550_sem* empty;
  empty = sem_mem;
  empty -> value = size;
  empty -> head = NULL;
  empty -> tail = NULL;

  //second semaphore to record used spaces
  struct cs1550_sem* full;
  full = sem_mem + 1;
  full -> value = 0;
  full -> head = NULL;
  full -> tail = NULL;

  //third semaphore to make sure mutual exclusion
  struct cs1550_sem* mutex;
  mutex = sem_mem + 2;
  mutex -> value = 1;
  mutex -> head = NULL;
  mutex -> tail = NULL;

  int* curr_produced = shared_mem;
  *curr_produced = 0;
  int* curr_consumed = shared_mem + 1;
  *curr_consumed = 0;
  int* buffer_ptr = shared_mem + 2;

  int i;	

  //these two for loops are similar to Dr. Misurda's lecture slides
  //producer
  for(i = 0; i < producer; i++){ 
    if(fork() == 0){
      int item;
      while(1){ 
	down(empty);//empty space one less
	down(mutex);//mutex down
	item = *curr_produced;
	buffer_ptr[*curr_produced % size] = item; 
	printf("Producer %c produced: Pancake%d\n", (i+65), item);
	*curr_produced += 1;
	up(mutex);//mutex up
	up(full);//full space one more
      }
    }
  }
  //consumer
  for(i = 0; i < consumer; i++){
    if(fork() == 0){
      int item;
      while(1){
	down(full);
	down(mutex);	
	item = buffer_ptr[*curr_consumed % size];
	printf("Consumer %c consumed: Pancake%d\n", (i+65), item);
	*curr_consumed += 1;
	up(mutex);
	up(empty);
      }
    }
  }
  int status;
  wait(&status);

  return 0;
}


