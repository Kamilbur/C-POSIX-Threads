#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#define DEBUG

#define READERS_COUNT 5
#define WRITERS_COUNT 5 
#define READER_TURNS 3
#define WRITER_TURNS 3
#define GetRandomTime(max) (rand() % max + 1)

#define SIZEOF_ARR(arr) (sizeof(arr) / sizeof(arr[0]))

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

#define SEM_POST(sem) do {  \
    if ( sem_post(&sem) ) { \
        err(EXIT_FAILURE,   \
        "Error occured during semaphore unlocking. [ine[%d]]",\
        __LINE__);          \
    }                       \
} while (0)

#define SEM_WAIT(sem) do {  \
    if ( sem_wait(&sem) ) { \
        err(EXIT_FAILURE,   \
        "Error occured during semaphore locking. [ine[%d]]",\
        __LINE__);          \
    }                       \
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
# define dprint(...) do {  \
} while (0)
#endif

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int buffers_read_max[] = {2, 3, 2};
const int buffers_num = SIZEOF_ARR(buffers_read_max);
pthread_mutex_t mutexesCountUpdate[SIZEOF_ARR(buffers_read_max)];
sem_t semaphoreWriters[SIZEOF_ARR(buffers_read_max)];
sem_t semaphoreReaders[SIZEOF_ARR(buffers_read_max)];

int Writer(void *data);
int Reader(void *data);

int
main(int argc, char *argv[])
{
    srand(100005);
    
    pthread_t writerThreads[WRITERS_COUNT];
    pthread_t readerThreads[READERS_COUNT];

    for (int ii = 0; ii < buffers_num; ii++) {
        pthread_mutex_init(mutexesCountUpdate + ii, NULL);
        sem_init(semaphoreWriters + ii, 0, 1);
        sem_init(semaphoreReaders + ii, 0, buffers_read_max[ii]);
    }

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

    // Wait for the Writers
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
    int buffer_idx;

    for (int ii = 0; ii < READER_TURNS; ii++) {
        for (;;) {
            buffer_idx = rand() % buffers_num;
            if ( sem_trywait(semaphoreReaders + buffer_idx) ) {
                if (errno != EAGAIN) {
                    err(EXIT_FAILURE,
                    "Error trying to decrement semaphore. Line[%d]",
                    __LINE__);
                }
            }
            else {
                dprint("(R) Reader %d succeded in decrementing semaphoreReaders[%d]\n", threadId, buffer_idx);
                MUTEX_LOCK(result, mutexesCountUpdate[buffer_idx]);
                if ( sem_getvalue(semaphoreReaders + buffer_idx, &result) ) {
                    err(EXIT_FAILURE,
                    "Error geting value from semaphore. Line[%d]",
                    __LINE__);
                }
                if (result == buffers_read_max[buffer_idx] - 1) {
                    SEM_WAIT(semaphoreWriters[buffer_idx]);
                    dprint("(R) Reader %d succeded in decrementing SemaphoreWriters[%d]\n", threadId, buffer_idx);
                }
                MUTEX_UNLOCK(result, mutexesCountUpdate[buffer_idx]);
                break;
            }
        }

        print("(R) Reader %d started reading from object[%d]\n", threadId, buffer_idx);

        // Read, read, read

	    usleep(GetRandomTime(200));

	    print("(R) Reader %d finished reading from object[%d]\n", threadId, buffer_idx);
            
	    // Unlock the semaphore object.

        MUTEX_LOCK(result, mutexesCountUpdate[buffer_idx]);
	    SEM_POST(semaphoreReaders[buffer_idx]);
        if ( sem_getvalue(semaphoreReaders + buffer_idx, &result) ) {
            err(EXIT_FAILURE,
            "Error geting value from semaphore. Line[%d]",
            __LINE__);
        }
        if (result == buffers_read_max[buffer_idx]) {
            SEM_POST(semaphoreWriters[buffer_idx]);
        }
        MUTEX_UNLOCK(result, mutexesCountUpdate[buffer_idx]);
        dprint("(R) Reader %d incremented semaphoreReaders[%d]\n", threadId, buffer_idx);

        usleep(GetRandomTime(800));
    }

    free(data);

    return (0);
}

int
Writer(void *data)
{
    int threadId = *(int*) data;
    int buffer_idx;
    
    for (int ii = 0; ii < WRITER_TURNS; ii++) {
        for (;;) {
            buffer_idx = rand() % buffers_num;                        
            if ( sem_trywait(semaphoreWriters + buffer_idx) ) {
                if (errno != EAGAIN) {
                    err(EXIT_FAILURE,
                    "Error trying to decrement semaphore. Line[%d]",
                    __LINE__);
                }
            }
            else {
                dprint("(W) Writer %d succeded in decrementing semaphoreWriters[%d]\n", threadId, buffer_idx);
                break;
            }
        }
	    print("(W) Writer %d started writing to object[%d]\n", threadId, buffer_idx);

        // Write, write, write
    	usleep(GetRandomTime(800));

    	print("(W) Writer %d finished writing to object[%d]\n", threadId, buffer_idx);
            
    	SEM_POST(semaphoreWriters[buffer_idx]);
        dprint("(W) Writer %d posted semaphoreWriters[%d]\n", threadId, buffer_idx);

        // Think, think, think, think
	    usleep(GetRandomTime(1000));
    }

    free(data);

    return (0);
}
