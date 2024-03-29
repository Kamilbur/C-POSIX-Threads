#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#define DEBUG

#define READERS_COUNT 5
#define WRITERS_COUNT 5
#define READER_TURNS 5
#define WRITER_TURNS 2
#define NUM_OF_SHARED_RESOURCE 5

#define GetRandomTime(max) (rand() % max + 1)

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

pthread_mutex_t mutexes[NUM_OF_SHARED_RESOURCE];

int Writer(void *data);
int Reader(void *data);

int
main(int argc, char *argv[])
{
    srand(100005);
    
    pthread_t writerThreads[WRITERS_COUNT];
    pthread_t readerThreads[READERS_COUNT];

    int rc;

    for (int ii = 0; ii < NUM_OF_SHARED_RESOURCE; ii++) {        
        pthread_mutex_init(mutexes + ii, NULL);
    }

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
    int shared_resource_idx;
    
    for (int ii = 0; ii < READER_TURNS; ii++) {
        do {
            shared_resource_idx = rand() % NUM_OF_SHARED_RESOURCE;

            dprint("(R) Reader %d tries to lock mutex[%d]\n", threadId, shared_resource_idx);
            result = pthread_mutex_trylock(mutexes + shared_resource_idx);
            if ( result && result != EBUSY) {
                errno = result;
                err(EXIT_FAILURE,
                    "Error during locking the mutex. Line[%d]",
                    __LINE__);
            }
            if ( result ) {
                dprint("(R) Reader %d did not succed in locking mutex[%d]\n", threadId, shared_resource_idx); 
            }
            else {
                dprint("(R) Reader %d succeded in locking mutex[%d]\n", threadId, shared_resource_idx); 
            }
        } while ( result );
        
        print("(R) Reader %d started reading from: %d\n", threadId, shared_resource_idx);

        // Read, read, read

    	usleep(GetRandomTime(200));
    	print("(R) Reader %d finished reading from: %d\n", threadId, shared_resource_idx);
    	// Release ownership of the mutex object.
        MUTEX_UNLOCK(result, mutexes[shared_resource_idx]);
        dprint("(R) Reader %d unlocked mutex[%d]\n", threadId, shared_resource_idx);

        
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
    int shared_resource_idx;
    
    for (int ii = 0; ii < WRITER_TURNS; ii++) {
        do {
            // Choose the resource to write to
            shared_resource_idx = rand() % NUM_OF_SHARED_RESOURCE;

            // Try locking mutex
            dprint("(W) Writer %d tries to lock mutex[%d]\n", threadId, shared_resource_idx);
            result = pthread_mutex_trylock(mutexes + shared_resource_idx);
            if (result && result != EBUSY) {
                errno = result;
                err(EXIT_FAILURE,
                    "Error during locking the mutex. Line[%d]",
                    __LINE__);
            }
            if ( result ) {
                dprint("(W) Writer %d did not succed in locking mutex[%d]\n", threadId, shared_resource_idx); 
            }
            else {
                dprint("(W) Writer %d succeded in locking mutex[%d]\n", threadId, shared_resource_idx); 
            }
        } while ( result );

    	print("(W) Writer %d started writing to: %d\n", threadId, shared_resource_idx);
 
        // Write, write, write
        usleep(GetRandomTime(800));

        print("(W) Writer %d finished writing to: %d\n", threadId, shared_resource_idx);
    	// Release ownership of the mutex object.
        MUTEX_UNLOCK(result, mutexes[shared_resource_idx]);
        dprint("(W) Writer %d unlocked mutex[%d]\n", threadId, shared_resource_idx);

        // Think, think, think, think
	    usleep(GetRandomTime(1000));
    }

    free(data);

    return (0);
}
