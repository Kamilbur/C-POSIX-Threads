#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#define DEBUG

#define BUFFER_SIZE 3
#define READERS_COUNT 3
#define WRITERS_COUNT 3
#define READER_TURNS 3
#define WRITER_TURNS 3
#define GetRandomTime(max) (rand() % max + 1)

#define MUTEX_LOCK(result_variable, lock) do {      \
    result_variable = pthread_mutex_lock(&lock);    \
    if ( result_variable ) {                        \
        errno = result_variable;                    \
        err(EXIT_FAILURE,                           \
        "Error occured during locking the mutex. Line[%d]",\
        __LINE__);                                  \
    }                                               \
} while (0)

#define MUTEX_UNLOCK(result_variable, lock) do {    \
    result_variable = pthread_mutex_unlock(&lock);  \
    if ( result_variable ) {                        \
        errno = result_variable;                    \
        err(EXIT_FAILURE,                           \
        "Error occured during unlocking the mutex. Line[%d]",\
        __LINE__);                                  \
    }                                               \
} while (0)

#define print(...) do {     \
    printf(__VA_ARGS__);    \
    fflush(stdout);         \
} while (0)


#ifdef DEBUG
# define dprint(...) do {       \
    print(__VA_ARGS__);         \
} while (0)
#else
# define dprint(msg, ...) do {  \
} while (0)
#endif

pthread_mutex_t mutexWriters = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condWriters = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexReaders = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condReaders = PTHREAD_COND_INITIALIZER;

volatile int buff_pos_read = BUFFER_SIZE;
volatile int buff_pos_write = 0;
volatile int buff_fill = 0;
volatile int buff_place;

int Writer(void *data);
int Reader(void *data);

int
main(int argc, char *argv[])
{
    srand(100005);
    
    pthread_t writerThreads[WRITERS_COUNT];
    pthread_t readerThreads[READERS_COUNT];

    int rc;

    // Create Writer threads
    for (int ii = 0; ii < WRITERS_COUNT; ii++) {
        usleep(GetRandomTime(1000));
        int *threadId = malloc(sizeof(int));
        *threadId = ii;
        rc = pthread_create(
                writerThreads + ii,  // thread identifier
                NULL,           // thread attributes
                (void*) Writer, // thread function
                (void*) threadId);  // thread function argument
        if (rc != 0) {
            errno = rc;
            err(EXIT_FAILURE, "Couldn't create the writer thread");
        }
    }

    // Create the Reader threads
    for (int ii = 0; ii < READERS_COUNT; ii++) {
	    // Reader initialization - takes random amount of time
    	usleep(GetRandomTime(1000));
        int *threadId = malloc(sizeof(int));
        *threadId = ii;
	    rc = pthread_create(
                readerThreads + ii, // thread identifier
                NULL,               // thread attributes
                (void*) Reader,     // thread function
                (void*) threadId);  // thread function argument

        if (rc != 0) {
            err(EXIT_FAILURE, "Couldn't create the reader threads");
        }
    }

    // At this point, the readers and writers should perform their operations

    // Wait for the Readers
    for (int ii = 0; ii < READERS_COUNT; ii++) { 
        pthread_join(readerThreads[ii], NULL);
    }

    // Wait for the Writer
    for (int ii = 0; ii < WRITERS_COUNT; ii++) {
        pthread_join(writerThreads[ii], NULL);
    }

    return (0);
}

int
Reader(void *data)
{
    int threadId = *(int*) data;
    int result;
    
    for (int ii = 0; ii < READER_TURNS; ii++) {
        dprint("(R) Reader %d trying to lock mutexReaders\n", threadId);
        MUTEX_LOCK(result, mutexReaders);
        dprint("(R) Reader %d obtained mutexReaders\n", threadId);
        while (buff_pos_read >= BUFFER_SIZE) {
            dprint("(R) Reader %d starting conditional wait\n", threadId);
            if ( (result = pthread_cond_wait(&condReaders, &mutexReaders)) ) {
                errno = result;
                err(EXIT_FAILURE,
                "Error during conditional wait. Line[%d]",
                __LINE__);
            }
        }

        buff_pos_read++;
        MUTEX_UNLOCK(result, mutexReaders);
        dprint("(R) Reader %d unlocked mutexReaders\n", threadId);


        print("(R) Reader %d started reading\n", threadId);

        // Read, read, read

	    usleep(GetRandomTime(200));
	    print("(R) Reader %d finished reading\n", threadId);


        dprint("(R) Reader %d trying to lock mutexReaders\n", threadId);
        MUTEX_LOCK(result, mutexReaders);
        dprint("(R) Reader %d obtained mutexReaders\n", threadId);

        if (++buff_place >= BUFFER_SIZE) {
            dprint("(R) Reader %d trying to lock mutexWriters\n", threadId);
            MUTEX_LOCK(result, mutexWriters);
            dprint("(R) Reader %d obtained mutexWriters\n", threadId);

            buff_pos_write = 0;
            if ( (result = pthread_cond_broadcast(&condWriters)) ) {
                errno = result;
                err(EXIT_FAILURE,
                "Error during condition broadcast. Line[%d]",
                __LINE__);
            }
            MUTEX_UNLOCK(result, mutexWriters);
            dprint("(R) Reader %d unlocked mutexWriters\n", threadId);

            buff_place = 0;
        }
        MUTEX_UNLOCK(result, mutexReaders);
        dprint("(R) Reader %d unlocked mutexWriters\n", threadId);
	    
        usleep(GetRandomTime(800));
    }

    free(data);

    return (0);
}

int
Writer(void *data)
{
    int threadId = *(int*) data;
    int result;
    
    for (int ii = 0; ii < WRITER_TURNS; ii++) {
        dprint("(W) Writer %d trying to lock mutexWriters\n", threadId);
        MUTEX_LOCK(result, mutexWriters);
        dprint("(W) Writer %d obtained mutexWriters\n", threadId);
        while (buff_pos_write >= BUFFER_SIZE) {
            dprint("(W) Writer %d starting conditinal wait\n", threadId);
            if ( (result = pthread_cond_wait(&condWriters, &mutexWriters)) ) {
                errno = result;
                err(EXIT_FAILURE,
                "Error conditional waiting. Line[%d]",
                __LINE__);
            }
        }
        // Set new position to write
        buff_pos_write++;
        MUTEX_UNLOCK(result, mutexWriters);
        dprint("(W) Writer %d unlocked mutexWriters\n", threadId);


	    print("(W) Writer %d started writing\n", threadId);
        
        // Write, write, write
    	usleep(GetRandomTime(800));

    	print("(W) Writer %d finished writing\n", threadId);
            

        dprint("(W) Writer %d trying to lock mutexWriters\n", threadId);
        MUTEX_LOCK(result, mutexWriters);
        dprint("(W) Writer %d obtained mutexWriters\n", threadId);
        if (++buff_fill >= BUFFER_SIZE) {
            MUTEX_LOCK(result, mutexReaders);
            buff_pos_read = 0;
            if ( (result = pthread_cond_broadcast(&condReaders)) ) {
                errno = result;
                err(EXIT_FAILURE,
                "Error during condition broadcast. Line[%d]",
                __LINE__);
            }
            MUTEX_UNLOCK(result, mutexReaders);
            buff_fill = 0;
        }

        MUTEX_UNLOCK(result, mutexWriters);
    	    
        // Think, think, think, think
	    usleep(GetRandomTime(1000));
    }

    return (0);
}
