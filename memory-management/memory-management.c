#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define LONG 2000
#define ITERATIONS 4000

//global variables
int memory[LONG];

int now;
int free_start = 0;

int allocation_count = 0;
int release_count = 0;

//event struct
typedef struct Event {

    int time;
    int type;

} Event;

//node for event linked list queue
struct Event_LLNode {

    Event data;
    struct Event_LLNode* next;

};

//front of priority queue
struct Event_LLNode* front;

//function prototypes
void enqueue(Event e);
void dequeue();
int allocate(int k);
void release(int header);
void print_statistics(); 

void main() {

    //initialize all memory to -2 (indicated free blocks) for debugging
    int mem_index = 0;

    while (mem_index < LONG) {

        memory[mem_index] = -2;
        mem_index++;
    }

    //Sentinel
    memory[0] = 0;
    memory[1] = 4;
    memory[2] = 4;
    memory[3] = 0;

    //Initial free block
    memory[4] = 4*(LONG - 5);
    memory[5] = 0;
    memory[6] = 0;
    memory[LONG - 2] = LONG - 5;

    //End of memory
    memory[LONG - 1] = 1;    

    //random number seed
    srand(time(NULL));
    
    int address, s, t, T, allocated_address;

    int free_header;

    //initial allocation event
    Event *new_event = (Event*) malloc(sizeof(Event));
    new_event->time = 0;
    new_event->type = -1;
    
    enqueue(*new_event);

    //run until 4000 allocations
    while (allocation_count < ITERATIONS) {

        //process event
        now = front->data.time;
        address = front->data.type;
        
        dequeue();
        
        //release event
        if (address >= 0) {

            release(address);

            //detail releases until 40 releases have occurred
            if (release_count < 40) {

                printf("Release    - Address: ");
                printf("%d\n", address);
            }

            release_count++;
        }    
        //allocation event     
        else {
        
            //random numbers for allocation
            s = (rand() % 100) + 1;
            t = (rand() % 250) + 1;
            T = (rand() % 60) + 1;

            allocated_address = allocate(s);

            //allocation failed
            if (allocated_address == -1) {

                printf("\nAllocation failed\n");
                printf("Failed Request Size: ");
                printf("%d\n", s);

                free_header = memory[1];
                printf("Size of All Free Blocks: ");

                //print sizes of all free blocks
                while (true) {

                    printf("%d", (int) memory[free_header] / 4);       
    
                    free_header = memory[free_header + 1];
                    
                    if (free_header == 0) {

                        printf("\n");
                        break;
                    }
                    printf(", ");
                }                

                print_statistics();         

                return;
            }
            //allocation successful
            else {  

                //detail allocations until 40 releases have occurred
                if (release_count < 40) {

                    printf("Allocation - Address: ");
    
                    //formatting
                    if (allocated_address < 10) {

                        printf("%d\t\t", allocated_address);
                    }
                    else {

                        printf("%d\t", allocated_address);
                    } 
                    printf(" Size: ");
                    printf("%d\t", s);
                    printf(" Finish Time: ");
                    printf("%d\t", now + t);
                    printf(" Next Arrival: ");
                    printf("%d\n", now + T);
                }
      
                //release event for this allocation
                new_event->time = now + t;
                new_event->type = allocated_address;
                enqueue(*new_event);   
                
                //next allocation event
                new_event->time = now + T;
                new_event->type = -1;
                enqueue(*new_event);
            }

            allocation_count++;

            //print statistics every 50 allocations
            if (allocation_count % 50 == 0) {
                
                print_statistics();
            }
        }
    }
}

//adds an element to the appropriate position in the priority queue
void enqueue(Event e) {

    struct Event_LLNode* new = (struct Event_LLNode*) malloc(sizeof(struct Event_LLNode));
    struct Event_LLNode* insertion_point = front;

    new->data = e;
    new->next = NULL;

    if (front == NULL || front->data.time >= new->data.time) {
    
        new->next = front;        
        front = new;
        return;
    }

    //find appropriate insertion point
    while (insertion_point->next != NULL && insertion_point->next->data.time < new->data.time) {

        insertion_point = insertion_point->next;
    }

    new->next = insertion_point->next;
    insertion_point->next = new;
}

//remove element from front of priority queue
void dequeue() {

    struct Event_LLNode* to_remove = front;

    if (front != NULL) {
    
        if (front == NULL) {

            front = NULL;
        }
        else {

            front = front->next;            
        }
    }

    free(to_remove);
}

//allocate job of requested k
int allocate(int k) {

    //avoid allocating a block of size less than 3
    if (k < 3) {

        k = 3;
    }

    //loop variables
    int current_free = free_start;
    int current_size = 0;
    int previous_f_link = 0;
    int next_b_link = 0;

    int mem_index;

    //look for big enough free block - break condition in loop
    while (true) {

        //size of current block and locations of free block links to update
        current_size = (int) memory[current_free] / 4;
        previous_f_link = memory[current_free + 2] + 1;
        next_b_link = memory[current_free + 1] + 2;

        //block found
        if (current_size >= k + 1) {

            //no split case
            if (current_size < k + 5) {

                //update free_start and free block links
                free_start = memory[current_free + 1];
                memory[previous_f_link] = memory[current_free + 1];
                memory[next_b_link] = memory[current_free + 2];  
                
                //update header of newly allocated block
                if (current_free == 4) {

                    memory[current_free] = 4*(current_size) + 1;
                }
                else {

                    memory[current_free] = 4*(current_size) + 2 + 1;
                }

                //set allocated block values to 2 for debugging
                mem_index = current_free + 1;

                while (mem_index < current_free + current_size) {

                    memory[mem_index] = 2;
                    mem_index++;    
                }   

                //update preuse of allocated block to the right
                if (current_free + current_size != LONG - 1) {
                    
                    memory[current_free + current_size] += 2;
                }
            }
            //split
            else {

                //update free_start and free block links
                free_start = current_free + k + 1;
                memory[previous_f_link] = current_free + k + 1;
                memory[next_b_link] = current_free + k + 1;

                //update header of newly allocated block
                if (current_free == 4) {

                   memory[current_free] = 4*(k + 1) + 1;
                }
                else {

                    memory[current_free] = 4*(k + 1) + 2 + 1;
                }

                //update info of new free block
                memory[current_free + k + 1] = 4*(current_size - (k + 1)) + 2;
                memory[current_free + k + 2] = memory[current_free + 1];
                memory[current_free + k + 3] = memory[current_free + 2];
                memory[current_free + current_size - 1] = current_size - (k + 1);

                //set free block values to -2 for debugging
                mem_index = current_free + 1;

                while (mem_index < current_free + k + 1) {

                    memory[mem_index] = 2;
                    mem_index++;    
                }
            }

            return current_free;   
        }

        //all free partitions checked - allocation not possible
        if (memory[current_free + 1] == free_start) {

            return -1;
        }
        //next free block
        current_free = memory[current_free + 1];
    }    
}

//release block with starting address 'header'
void release(int header) {

    //determine if adjacent blocks are free/allocated
    int left_use = (int) (memory[header] % 4) / 2;
    int right_use = memory[header + ((int) memory[header] / 4)] % 2;
    
    //handle special case where block to left is the sentinel
    if (header == 4) {

        left_use = 1;
    }

    //size of block to be released
    int release_size = (int) memory[header] / 4; 

    //size of blocks to left and right of released block, if such blocks are free   
    int left_size = memory[header - 1];
    int right_size = (int) memory[header + release_size] / 4;

    int previous_header;
    int next_header;

    int mem_index;

    //left and right allocated case
    if (left_use == 1 && right_use == 1) {

        //update new free block's info
        memory[header] -= 1;
        memory[header + 1] = 0;
        memory[header + 2] = memory[2];
        memory[header + release_size - 1] = release_size;

        //update links to new free block
        memory[memory[2] + 1] = header;
        memory[2] = header;

        //update preuse of right block 
        if (header + release_size < 1999) {

            memory[header + release_size] -= 2;
        } 

        //set free block values to -2 for debugging
        mem_index = header + 3;

        while (mem_index < header + release_size - 1) {

            memory[mem_index] = -2;
            mem_index++;
        }                 
    }
    //Only left allocated case
    else if (left_use == 1) {
        
        //update new free block's info
        memory[header] = 4*(release_size + right_size) + 2;
        memory[header + 1] = memory[header + release_size + 1];
        memory[header + 2] = memory[header + release_size + 2];
        memory[header + release_size + right_size - 1] = release_size + right_size;

        //update links to updated free block
        memory[memory[header + 1] + 2] = header;
        memory[memory[header + 2] + 1] = header;

        //set free block values to -2 for debugging
        mem_index = header + 3;

        while (mem_index < header + release_size + right_size - 1) {

            memory[mem_index] = -2;
            mem_index++;
        }  
    }
    //Only right allocated case
    else if (right_use == 1) {
        
        if (left_size == 0) {

            memory[header] = 4*(release_size + left_size);
        }
        else {

            memory[header - left_size] = 4*(release_size + left_size) + 2;
        }

        memory[header + release_size - 1] = release_size + left_size;

        //update preuse of right block 
        if (header + release_size < 1999) {

            memory[header + release_size] -= 2;
        } 
        
        //set free block values to -2 for debugging
        mem_index = header - left_size + 3;

        while (mem_index < header + release_size - 1) {

            memory[mem_index] = -2;
            mem_index++;
        }  
    }
    //Left and right free case
    else {
        
        previous_header = memory[header + release_size + 2];
        next_header = memory[header + release_size + 1];

        //new header info
        if (left_size == 0) {
        
            memory[header] = 4*(release_size + right_size);
        }
        else {

            memory[header - left_size] = 4*(release_size + left_size + right_size) + 2;
        }        

        memory[header + release_size + right_size - 1] = release_size + left_size + right_size;

        memory[next_header + 2] = previous_header;
        memory[previous_header + 1] = next_header;    

        //set free block values to -2 for debugging
        mem_index = header - left_size + 3;

        while (mem_index < header + release_size + right_size - 1) {

            memory[mem_index] = -2;
            mem_index++;
        }         
    }
}

//print various statistics about state of memory
void print_statistics() {

    //used for interation through memory blocks
    int header3 = 4;
    int header2 = 0;

    //statistics variables
    int free_count = 0;
    int allocated_count = 0;
    int total_free_size = 0;
    int total_allocated_size = 0;
    float avg_free_size;
    float avg_allocated_size;
    int k = 0;
    int free_not_allocated; 

    //iterate through memory blocks
    while (header3 < LONG - 1) {
    
        //free block
        if (memory[header3] % 2 == 0) {

            free_count++;
            total_free_size += (int) memory[header3] / 4;
        }
        //allocated block
        else {
            
            allocated_count++;
            total_allocated_size += (int) memory[header3] / 4;
        }

        //next block
        header3 += (int) memory[header3] / 4;
    }

    if (free_count > 0) {

        avg_free_size = (float) total_free_size / free_count;
    }
    else {

        avg_free_size = 0;    
    }

    if (allocated_count > 0) {

        avg_allocated_size = (float) total_allocated_size / allocated_count;
    }
    else {

        avg_allocated_size = 0;
    }

    //iterate through free blocks
    while (true) {

        //free block larger than average allocated block 
        if (((int) memory[header2] / 4) > avg_allocated_size) {

            k++;
        }

        header2 = memory[header2 + 1];
        
        //sentinel - break case
        if (header2 == 0) {

            break;
        }
    }
    
    //compute fragmentation percentage variable
    if (total_free_size > 0) {

        free_not_allocated = 100 * (1 - (k*avg_allocated_size) / total_free_size);
    }
    else {

        free_not_allocated = -1;
    }

    //print statistics
    printf("\n#Stats\n");
    printf("Simulated Time: ");
    printf("%d\n", now);
    printf("Allocations: ");
    printf("%d\n", allocation_count);
    printf("Releases: ");
    printf("%d\n", release_count);
    printf("Free Blocks: ");
    printf("%d\n", free_count);
    printf("Allocated Blocks: ");
    printf("%d\n", allocated_count);
    printf("Total Free: ");
    printf("%d\n", total_free_size);
    printf("Total Allocated: ");
    printf("%d\n", total_allocated_size);
    printf("Average Free: ");
    printf("%.2f\n", avg_free_size);
    printf("Average Allocated: ");
    printf("%.2f\n", avg_allocated_size);
    printf("Possible Average Requests: ");
    printf("%d\n", k);
    printf("Percentage of Free Memory Not Potentially Allocated: ");

    if (free_not_allocated >= 0) {
   
        printf("%d\n", free_not_allocated);
    }
    else {

        printf("No Free Memory\n");
    }
    printf("\n");
}
