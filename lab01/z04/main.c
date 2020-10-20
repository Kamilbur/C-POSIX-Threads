#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define READERS_COUNT 5
#define WRITERS_COUNT 3
#define READER_TURNS 5
#define WRITER_TURNS 2
#define write_happened() (writeHappened = 1)
#define write_handled()  (writeHappened = 0)

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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t writeHappenedCond = PTHREAD_COND_INITIALIZER;

volatile int writeHappened = 0;

int Writer(void *data);
int Reader(void *data);
int Critic(void *data);

int
main(int argc, char *argv[])
{
    srand(100005);
    
    pthread_t writerThreads[WRITERS_COUNT];
    pthread_t readerThreads[READERS_COUNT];
    pthread_t criticThread;

    int rc;

    // Create Critic thread
    rc = pthread_create(
                &criticThread,
                NULL,
                (void *) Critic,
                NULL);
    if ( rc ) {
        errno = rc;
        err(EXIT_FAILURE, "Couldn't create the critic thread");
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
    
    for (int ii = 0; ii < READER_TURNS; ii++) {
        MUTEX_LOCK(result, mutex);
        printf("(R) Reader %d started reading...", threadId);
	    fflush(stdout);

        // Read, read, read

	    usleep(GetRandomTime(200));
	    printf("finished\n");
        fflush(stdout);

	    // Release ownership of the mutex object.
        MUTEX_UNLOCK(result, mutex);
	    
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
        MUTEX_LOCK(result, mutex);

        while ( writeHappened ) {
            result = pthread_cond_wait(&writeHappenedCond, &mutex);
            if ( result ) {
                errno = result;
                err(EXIT_FAILURE,
                    "Waiting for condition variable error. Line [%d]",
                    __LINE__);
            }
        }

	    printf("(W) Writer %d started writing...", threadId);
	    fflush(stdout);

        // Write, write, write
    	usleep(GetRandomTime(800));

    	printf("finished\n");
        fflush(stdout);

        // Inform critic, that write happened
        write_happened();
        result = pthread_cond_signal(&writeHappenedCond);
        if ( result ) {
            errno = result;
            err(EXIT_FAILURE,
                "Condition signaling error. Line [%d]",
                __LINE__);
        }

	    // Release ownership of the mutex object.
        MUTEX_UNLOCK(result, mutex);
    	    
        // Think, think, think, think
	    usleep(GetRandomTime(1000));
    }

    return (0);
}

int
Critic(void *data)
{
    int numOfActions = WRITERS_COUNT * WRITER_TURNS;    
    int result;

    while (numOfActions) {
        MUTEX_LOCK(result, mutex);
        while ( !writeHappened ) {
            result = pthread_cond_wait(&writeHappenedCond, &mutex);
            if ( result ) {
                errno = result;
                err(EXIT_FAILURE, "Waiting for condition variable error.");
            }
        }

        printf("(C) Critic started criticising\n");
        fflush(stdout);

        // Criticise, criticise, criticies
        usleep(GetRandomTime(500));

        printf("(C) Critic finished criticising\n");
        fflush(stdout);

        // Inform writers, that critic finished
        write_handled();
        numOfActions--;

        result = pthread_cond_signal(&writeHappenedCond);
        if ( result ) {
            errno = result;
            err(EXIT_FAILURE,
                "Condition signaling error. Line [%d]",
                __LINE__);
        }

        MUTEX_UNLOCK(result, mutex);
    }

    return (0);
}
