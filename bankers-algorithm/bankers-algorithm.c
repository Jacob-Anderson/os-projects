#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//The ids of the processes in the rows of each matrix
int matrix_ids[5];

//The available vector and matrices used for the banker's algorithm
int available[4] = {13, 10, 7, 12};
int max[5][4], alloc[5][4], need[5][4];

//Instruction struct (Request, Release, Sleep..)
typedef struct Instruction {

    char type;
    int r_values[4];

} Instruction;

//Used for Instruction linked-list queues
struct Instruction_LLNode {

    Instruction data;
    struct Instruction_LLNode* next;

};

//Process struct
typedef struct Process {
        
    int id;
    int max_need_values[4];
    struct Instruction_LLNode* instructions_front;
    struct Instruction_LLNode* instructions_rear;
    
} Process;

//Used for Process linked-list queues
struct Process_LLNode {

    Process data;
    struct Process_LLNode* next;

};

//Queues
struct Process_LLNode* outer_front;
struct Process_LLNode* outer_rear;

struct Process_LLNode* ready_front;
struct Process_LLNode* ready_rear;

struct Process_LLNode* wait_front;
struct Process_LLNode* wait_rear;

//function prototypes
void process_input();
void enqueue_process(char queue, Process pro);
void dequeue_process(char queue);
void add_instruction(Process *pro, Instruction ins);
void remove_instruction(Process *p);
int get_process_row(int id);
bool allocate(Process p, int r1, int r2, int r3, int r4);
void release(Process p, int r1, int r2, int r3, int r4);
bool is_safe();
void print_matrices();
void update_need();
void execute_instruction(Process *p); 
void swap_process(); 
void empty_wait();

void main() {

    //initialize allocation matrix
    int r = 0;
    int c = 0;

    while (r < 5) {

        while (c < 4) {

            alloc[r][c] = 0;
            c++;
        }  
        r++;  
    }

    //create Processes and load into queues (5 in ready, the rest in outer)
    process_input();

    //execute the first instruction of the "running" process until there are none
    while (ready_front != NULL) {

        execute_instruction(&(ready_front->data));
    }
}

//create Proccesses, load into queues
//initialize matrices for banker's algorithm
void process_input() {

    bool done = false;
    int chars_read;
    char read_char;
    char line[64];
    int count = 0;

    int count2 = 0;
    char num_string[16];

    int count3 = 0;
    int count4 = 0;

    Process *new_process = (Process*) malloc(sizeof(Process));
    Instruction *new_instruction;
    
    while (!done) {

        chars_read = read(0, &read_char, 1);
        
        //EOF condition
        if (chars_read <= 0) {
        
            done = true;
            continue;
        }

        if (read_char != '\n') {
    
            line[count] = read_char;
            count++;
        }
        else {

            line[count] = '\0';
            
            //ID line
            if (line[0] == 'I' && line[1] == 'D') {
            
                count2 = 3;
                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_process->id = atoi(num_string);
            }
            //Max need line
            else if (line[0] == 'M' && line[1] == 'N') {

                count2 = 3;
                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_process->max_need_values[0] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_process->max_need_values[1] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_process->max_need_values[2] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_process->max_need_values[3] = atoi(num_string);              
            }
            //Sleep line
            else if (line[0] == 'S' && line[1] == 'L') {
                
                new_instruction = (Instruction*) malloc(sizeof(Instruction));
                new_instruction->type = 'S';
                
                add_instruction(new_process, *new_instruction);                

            }
            //Request line
            else if (line[0] == 'R' && line[1] == 'Q') {

                new_instruction = (Instruction*) malloc(sizeof(Instruction));
                new_instruction->type = 'Q';
    
                count2 = 3;
                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_instruction->r_values[0] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_instruction->r_values[1] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_instruction->r_values[2] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_instruction->r_values[3] = atoi(num_string);

                add_instruction(new_process, *new_instruction); 
            }   
            //Release line
            else if (line[0] == 'R' && line[1] == 'L') {

                new_instruction = (Instruction*) malloc(sizeof(Instruction));
                new_instruction->type = 'L';

                count2 = 3;
                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_instruction->r_values[0] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_instruction->r_values[1] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_instruction->r_values[2] = atoi(num_string);

                count3 = 0;

                while (isspace(line[count2])) {
            
                    count2++;
                }

                while (!isspace(line[count2])) {
        
                    num_string[count3] = line[count2];
                    count3++;
                    count2++;
                }

                num_string[count3] = '\0';            

                new_instruction->r_values[3] = atoi(num_string);

                add_instruction(new_process, *new_instruction);
            }
            //End line
            else if (line[0] == 'E' && line[1] == 'N' && line[2] == 'D') {         

                if (count4 < 5) {
                
                    matrix_ids[count4] = new_process->id;
                    max[count4][0] = new_process->max_need_values[0];
                    max[count4][1] = new_process->max_need_values[1];
                    max[count4][2] = new_process->max_need_values[2];
                    max[count4][3] = new_process->max_need_values[3];
                    enqueue_process('R', *new_process);
                    count4++;
                }
                else {
            
                    enqueue_process('O', *new_process);
                }

                new_process = (Process*) malloc(sizeof(Process));   
            }

            count = 0;
        }
    }
    update_need();
}

//enqueue a proccess into a specified queue
void enqueue_process(char queue, Process pro) {

    struct Process_LLNode* new = (struct Process_LLNode*) malloc(sizeof(struct Process_LLNode));

    new->data = pro;
    new->next = NULL;

    //outer queue
    if (queue == 'O') {

        if (outer_front == NULL && outer_rear == NULL) {
        
            outer_front = new;
            outer_rear = new;
            return;
        }

        outer_rear->next = new;
        outer_rear = new;
    }
    //ready queue
    else if (queue == 'R') {
        
        if (ready_front == NULL && ready_rear == NULL) {
        
            ready_front = new;
            ready_rear = new;
            return;
        }

        ready_rear->next = new;
        ready_rear = new;
    }
    //wait queue
    else if (queue == 'W') {
        
        if (wait_front == NULL && wait_rear == NULL) {
        
            wait_front = new;
            wait_rear = new;
            return;
        }

        wait_rear->next = new;
        wait_rear = new;
    }
    else {

        printf("Invalid Queue Designated");
    }
}

//dequeue a process from a specified queue
void dequeue_process(char queue) {
    
    //outer queue
    if (queue == 'O') {

        struct Process_LLNode* to_remove = outer_front;

        if (outer_front != NULL) {
        
            if (outer_front == outer_rear) {

                outer_front = NULL;
                outer_rear = NULL;
            }
            else {

                outer_front = outer_front->next;            
            }
        }

        free(to_remove);
    }
    //ready queue
    else if (queue == 'R') {

        struct Process_LLNode* to_remove = ready_front;

        if (ready_front != NULL) {
        
            if (ready_front == ready_rear) {

                ready_front = NULL;
                ready_rear = NULL;
            }
            else {

                ready_front = ready_front->next;            
            }
        }

        free(to_remove);
    }
    //wait queue
    else if (queue == 'W') {

        struct Process_LLNode* to_remove = wait_front;

        if (wait_front != NULL) {
        
            if (wait_front == wait_rear) {

                wait_front = NULL;
                wait_rear = NULL;
            }
            else {

                wait_front = wait_front->next;            
            }
        }

        free(to_remove);
    }
    else {

        printf("Invalid Queue Designated");   
    }
}

//add an instruction to a specified process's instruction queue
void add_instruction(Process *pro, Instruction ins) {

    struct Instruction_LLNode* new = (struct Instruction_LLNode*) malloc(sizeof(struct Instruction_LLNode));

    new->data = ins;
    new->next = NULL;

    if (pro->instructions_front == NULL && pro->instructions_rear == NULL) {
        
        pro->instructions_front = new;
        pro->instructions_rear = new;
        return;
    }

    pro->instructions_rear->next = new;
    pro->instructions_rear = new;
}

//dequeue an instruction from a specified process's instruction queue
void remove_instruction(Process *process) {

    struct Instruction_LLNode* to_remove = process->instructions_front;
    
    if (process->instructions_front != NULL) {

        if (process->instructions_front == process->instructions_rear) {
            
            process->instructions_front = NULL;
            process->instructions_rear = NULL;
        }
        else {
            
            process->instructions_front = process->instructions_front->next;
        }
    }
    
    free(to_remove);
}

//return the matrix row where the specified process (by id) can be found
int get_process_row(int id) {

    if (matrix_ids[0] == id) {

        return 0;
    }
    else if (matrix_ids[1] == id) {

        return 1;
    }
    else if (matrix_ids[2] == id) {

        return 2;
    }
    else if (matrix_ids[3] == id) {

        return 3;
    }
    else if (matrix_ids[4] == id) {

        return 4;
    }
    else {
        
        return -1;
    }
}

//allocate specified resources to the specified process
bool allocate(Process p, int r1, int r2, int r3, int r4) {
    
    int row = get_process_row(p.id);

    //illegal request case
    if (r1 < 0 || r2 < 0 || r3 < 0 || r4 < 0 
        || r1 > max[row][0] - alloc[row][0] || r2 > max[row][1] - alloc[row][1]
        || r3 > max[row][2] - alloc[row][2] || r4 > max[row][3] - alloc[row][3]) {
        
        swap_process();

        empty_wait();

        return false;
    }
    //unsafe request - not enough available resources to satisfy request
    else if (r1 > available[0] || r2 > available[1] 
             || r3 > available[2] || r4 > available[3]) {
    
        enqueue_process('W', ready_front->data);
        dequeue_process('R');

        printf("Request of job No. ");
        printf("%d", p.id);
        printf(" for resources: ");
        printf("\t%d\t", r1);
        printf("%d\t", r2);
        printf("%d\t", r3);
        printf("%d\t", r4);
        printf(" cannot be satisfied\n\n");

        print_matrices();

        return false;
    }
    else {
            
        available[0] -= r1;
        available[1] -= r2;
        available[2] -= r3;
        available[3] -= r4;

        alloc[row][0] += r1;
        alloc[row][1] += r2;
        alloc[row][2] += r3;
        alloc[row][3] += r4; 

        update_need();

        //unsafe request - banker's algorithm determined
        if (!is_safe()) {

            //move to wait queue
            enqueue_process('W', ready_front->data);
            dequeue_process('R');

            //print output
            printf("Request of job No. ");
            printf("%d", p.id);
            printf(" for resources: ");
            printf("\t%d\t", r1);
            printf("%d\t", r2);
            printf("%d\t", r3);
            printf("%d\t", r4);
            printf(" cannot be satisfied\n\n");

            //rollback allocation
            available[0] += r1;
            available[1] += r2;
            available[2] += r3;
            available[3] += r4;

            alloc[row][0] -= r1;
            alloc[row][1] -= r2;
            alloc[row][2] -= r3;
            alloc[row][3] -= r4; 

            update_need();
        
            //more output
            print_matrices();    

            return false;
        }  
        //safe request 
        else {
        
            return true;
        }
    }
}

//release specified resources from the specified process
void release(Process p, int r1, int r2, int r3, int r4) {

    int row = get_process_row(p.id);    

    //illegal release
    if (r1 < 0 || r2 < 0 || r3 < 0 || r4 < 0 
        || r1 > alloc[row][0] || r2 > alloc[row][1] 
        || r3 > alloc[row][2] || r4 > alloc[row][3]) {

        swap_process();
    }
    //legal release
    else {
    
        available[0] += r1;
        available[1] += r2;
        available[2] += r3;
        available[3] += r4;

        alloc[row][0] -= r1;
        alloc[row][1] -= r2;
        alloc[row][2] -= r3;
        alloc[row][3] -= r4;
    }

    update_need();
    
    //update wait/ready queues
    empty_wait();
}

//determine using banker's algorithm if the system is in a safe state
bool is_safe() {

    int local_available[4];
   
    local_available[0] = available[0];
    local_available[1] = available[1];
    local_available[2] = available[2];
    local_available[3] = available[3];

    bool bools[5] = {false, false, false, false, false};

    int j = 0;
    int i = 0;    

    //5 iterations necessary in worst case
    for (j = 0; j < 5; j++) {

        //iterate over all processes
        for (i = 0; i < 5; i++) {

            //iterate over only unsatisfied processes
            if (bools[i] == false) {
    
                //process can be satisfied
                if (need[i][0] <= local_available[0] 
                 && need[i][1] <= local_available[1]
                 && need[i][2] <= local_available[2]
                 && need[i][3] <= local_available[3]) {
    
                    bools[i] = true;
                    
                    local_available[0] += alloc[i][0];
                    local_available[1] += alloc[i][1];
                    local_available[2] += alloc[i][2];
                    local_available[3] += alloc[i][3];
                }
            }
        }

        //if all false at end of any iteration, unsafe
        if (bools[0] == false && bools[1] == false && bools[2] == false
            && bools[3] == false && bools[4] == false) {

            return false;
        }

        //if all true at any point, safe
        if (bools[0] == true && bools[1] == true && bools[2] == true
            && bools[3] == true && bools[4] == true) {

            return true;
        }
    }
    
    //otherwise, unsafe
    return false;    
}

//output the contents of the max, allocation, and need matrices and the 
//available vector 
void print_matrices() {

    printf("            AVAILABLE\n\n");
    printf("R1\t");
    printf("R2\t");
    printf("R3\t");
    printf("R4\n");
    printf("----------------------------------\n");
    printf("%d\t", available[0]);
    printf("%d\t", available[1]);
    printf("%d\t", available[2]);
    printf("%d\n\n\n", available[3]);

    printf("              ALLOC\n\n");
    printf("ID\t");
    printf("R1\t");
    printf("R2\t");
    printf("R3\t");
    printf("R4\n");
    printf("----------------------------------\n");

    int r = 0;
    int c = 0;

    while (r < 5) {

        c = 0;
        printf("%d\t", matrix_ids[r]);

        while (c < 4) {

            printf("%d\t", alloc[r][c]);
            c++;
        }
        printf("\n");  
        r++;  
    }
    printf("\n\n");

    printf("              MAX\n\n");
    printf("ID\t");
    printf("R1\t");
    printf("R2\t");
    printf("R3\t");
    printf("R4\n");
    printf("----------------------------------\n");

    r = 0;
    c = 0;

    while (r < 5) {

        c = 0;
        printf("%d\t", matrix_ids[r]);

        while (c < 4) {

            printf("%d\t", max[r][c]);
            c++;
        }
        printf("\n");  
        r++;  
    }
    printf("\n\n");   

    printf("              NEED\n\n");
    printf("ID\t");
    printf("R1\t");
    printf("R2\t");
    printf("R3\t");
    printf("R4\n");
    printf("----------------------------------\n");

    r = 0;
    c = 0;

    while (r < 5) {

        c = 0;
        printf("%d\t", matrix_ids[r]);

        while (c < 4) {

            printf("%d\t", need[r][c]);
            c++;
        }
        printf("\n");  
        r++;  
    }
    printf("\n\n");      
}

//update the contents of the need matrix using the max and allocation matrices
void update_need() {
    
    int r = 0;
    int c = 0;

    while (r < 5) {

        c = 0;
        while (c < 4) {

            need[r][c] = max[r][c] - alloc[r][c];
            c++;
        }
        r++;  
    }
}

//execute the next pending instruction of a specified process
void execute_instruction(Process *p) {

    int temp1, temp2, temp3, temp4;
    bool allocated;

    //no instructions, terminate process
    if (p->instructions_front == NULL) {

        swap_process();
         
        empty_wait();

        return;
    }

    //sleep instruction
    if (p->instructions_front->data.type == 'S') {

        remove_instruction(p);

        enqueue_process('R', ready_front->data);
        dequeue_process('R');
    }
    //release instruction
    else if (p->instructions_front->data.type == 'L') {

        temp1 = p->instructions_front->data.r_values[0];
        temp2 = p->instructions_front->data.r_values[1];
        temp3 = p->instructions_front->data.r_values[2];
        temp4 = p->instructions_front->data.r_values[3];

        remove_instruction(p);

        release(*p, temp1, temp2, temp3, temp4);

    }
    //request instruction
    else if (p->instructions_front->data.type == 'Q') {

        temp1 = p->instructions_front->data.r_values[0];
        temp2 = p->instructions_front->data.r_values[1];
        temp3 = p->instructions_front->data.r_values[2];
        temp4 = p->instructions_front->data.r_values[3];

        //only remove instruction if allocation was successful
        if (allocate(*p, temp1, temp2, temp3, temp4)) {
    
           remove_instruction(p); 
        }
    }
    else {
        
        printf("\n\nUNKNOWN INSTRUCTION\n\n");
        return;
    }
}

//terminate the "running" process, add the first process in the outer
//queue to the ready queue and update the max, allocation, and need matrices
void swap_process() {

    int row = get_process_row(ready_front->data.id);    

    dequeue_process('R');
        
    if (outer_front != NULL) {       
      
        matrix_ids[row] = outer_front->data.id;

        available[0] += alloc[row][0];
        available[1] += alloc[row][1];
        available[2] += alloc[row][2];
        available[3] += alloc[row][3];

        max[row][0] = outer_front->data.max_need_values[0];
        max[row][1] = outer_front->data.max_need_values[1];
        max[row][2] = outer_front->data.max_need_values[2];
        max[row][3] = outer_front->data.max_need_values[3];
    
        alloc[row][0] = 0;
        alloc[row][1] = 0;
        alloc[row][2] = 0;
        alloc[row][3] = 0;

        update_need();  

        enqueue_process('R', outer_front->data);
        dequeue_process('O');   
    }
}

//move the contents of the wait queue to the ready queue
//used whenever available resources are increased in system
void empty_wait() {

    struct Process_LLNode* to_move = wait_front;

    while (to_move != NULL) {

        enqueue_process('R', to_move->data);
        dequeue_process('W');     

        to_move = wait_front;       
    }

    free(to_move);
}
