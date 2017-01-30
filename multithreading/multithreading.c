#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

//special operations defined for buffer modification
#define BUFFER_LENGTH 256
#define ERASE_LAST_CHAR_OP '*'
#define INSERT_EOL_CHAR_OP '@'
#define DELETE_LAST_WORD_OP '$'
#define REMOVE_CURRENT_LINE_OP '&'

//end of line constant
#define EOL '\n'

//pthread variables for synchronization
pthread_mutex_t read_store_mutex, store_print_mutex;
pthread_cond_t read_cond, store_cond, store2_cond, print_cond;

//threads exit condition
bool done = false;

//global char for passing data from read thread to store thread
//initially null terminating character, indicating no character ready for storing
char read_store_char = '\0';

//global variables for storing characters passed to store thread 
char buffer[BUFFER_LENGTH];
int buff_size = 0;

//global bool for controlling store/print timing
//initially true, indicating writing to buffer can occur initially
bool allow_buffer_writing = true;

//function prototypes
void* read_func();
void* store_func();
void* print_func();
int size_after_word_delete();

/*
creates necessary mutexes, condition variables, and pthreads for program execution
waits on all threads to complete, then performs necessary clean-up  
*/
void main() {
    
    pthread_t read_thread, store_thread, print_thread;

    pthread_mutex_init(&read_store_mutex, NULL);
    pthread_mutex_init(&store_print_mutex, NULL);

    pthread_cond_init(&read_cond, NULL);
    pthread_cond_init(&store_cond, NULL);
    pthread_cond_init(&store2_cond, NULL);
    pthread_cond_init(&print_cond, NULL);

    pthread_create(&read_thread, NULL, read_func, NULL);
    pthread_create(&store_thread, NULL, store_func, NULL);
    pthread_create(&print_thread, NULL, print_func, NULL);

    pthread_join(read_thread, NULL);
    pthread_join(store_thread, NULL);
    pthread_join(print_thread, NULL);

    pthread_mutex_destroy(&read_store_mutex);
    pthread_mutex_destroy(&store_print_mutex);
    pthread_cond_destroy(&read_cond);
    pthread_cond_destroy(&store_cond);
    pthread_cond_destroy(&store2_cond);
    pthread_cond_destroy(&print_cond);
}

/*
reads 1 character at a time and sends it to the store thread using 
the global 'read_store_char' variable.
*/
void* read_func() {
    
    int chars_read;
    char local_char; 

    while (!done) {
    
        chars_read = read(0, &local_char, 1);

        //program, all threads end condition
        if (chars_read <= 0) {

            done = true;
            continue;
        }
        
        //critical section for access to global 'read_store_char'
        pthread_mutex_lock(&read_store_mutex);

        //wait while the previously read character has not been 'received' by the store thread
        while (read_store_char != '\0') {

            pthread_cond_wait(&read_cond, &read_store_mutex);
        }
        //'send' character to store thread
        read_store_char = local_char; 
       
        //signal store thread that a new character is ready for processing
        pthread_cond_signal(&store_cond);

        pthread_mutex_unlock(&read_store_mutex);
        //end critical section
    }
    pthread_exit(0);
}

/*
processes/stores the character 'received' from the read thread.
*/
void* store_func() {

    char local_char;

    while (!done) {

        //critical section for access to global 'read_store_char'
        pthread_mutex_lock(&read_store_mutex);
        
        //wait while the read thread has not 'sent' a new character
        while (read_store_char == '\0') {

            pthread_cond_wait(&store_cond, &read_store_mutex);
        }
        //'receive' character 'sent' by read thread
        local_char = read_store_char;       

        //indicate that global variable is empty and waiting for a new character
        read_store_char = '\0'; 
        
        //signal read thread that previous character has been 'received'
        pthread_cond_signal(&read_cond);
        
        pthread_mutex_unlock(&read_store_mutex);
        //end critical section

        //critical section for access to global buffer
        pthread_mutex_lock(&store_print_mutex);

        //wait while buffer is being accessed by print thread
        while (!allow_buffer_writing) {

            pthread_cond_wait(&store2_cond, &store_print_mutex);
        }       

        //process 'received' char
        switch (local_char) {
                
            case ERASE_LAST_CHAR_OP :
                //decrement size of buffer if buffer isn't empty
                if (buff_size > 0) {
                    
                    buff_size--;
                }
                break;

            case INSERT_EOL_CHAR_OP :
                //add eol character to end of buffer
                buffer[buff_size] = EOL;
                buff_size++;                
         
                //allow print thread to access buffer
                allow_buffer_writing = false;
                pthread_cond_signal(&print_cond);
                break;

            case DELETE_LAST_WORD_OP :
                //call function to determine new buffer size
                buff_size = size_after_word_delete();
                break;

            case REMOVE_CURRENT_LINE_OP :
                //clear buffer
                buff_size = 0;
                break;

            case EOL :
                //add eol character to end of buffer
                buffer[buff_size] = EOL;
                buff_size++; 
                   
                //allow print thread to access buffer
                allow_buffer_writing = false;
                pthread_cond_signal(&print_cond);
                break;

            default : 
                //store 'received' character in buffer                
                buffer[buff_size] = local_char;
                buff_size++;
                break;
        }

        pthread_mutex_unlock(&store_print_mutex);
        //end critical section
    }
    pthread_exit(0);
}

/*
print contents of buffer
*/
void* print_func() {

    int i;

    while (!done) {    

        //critical section for access to global buffer
        pthread_mutex_lock(&store_print_mutex);

        //wait while store thread is writing to buffer
        while (allow_buffer_writing) {
    
            pthread_cond_wait(&print_cond, &store_print_mutex);
        }

        i = 0;
        
        //print contents of buffer
        while (i < buff_size) {
        
            printf("%c", buffer[i]);
            i++;
        } 
        
        //clear buffer
        buff_size = 0;

        //indicate printing is done and buffer can be written to
        allow_buffer_writing = true;

        //signal store thread that printing is concluded
        pthread_cond_signal(&store2_cond);

        pthread_mutex_unlock(&store_print_mutex);
        //end critical section
    }
}

/*
determines size of buffer after deleting the last 'word' it contains
*/
int size_after_word_delete() {
    
    int new_size = buff_size;

    //ignore trailing whitespace
    while ((new_size > 0) & (buffer[new_size - 1] == ' ')) {
        
        new_size--;
    }

    //if buffer contains only whitespace, no word to delete
    if (new_size == 0) {

        return buff_size;
    }

    //ignore characters until next whitespace or beginning of buffer
    while ((new_size > 0) & (buffer[new_size - 1] != ' ')) {
        
        new_size--;
    }

    return new_size;
}
