#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define READERS_COUNT 5
#define WRITERS_COUNT 5
#define READER_TURNS 5
#define WRITER_TURNS 2
#define BUFFER_SIZE 3
#define BUFFER_EMPTY_INTERVAL 10000
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


pthread_mutex_t writerCondLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readersCountUpdate = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexWriters = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bufferSpaceUpdate = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condWriter = PTHREAD_COND_INITIALIZER;

int readersCount = 0;
int bufferSpace = BUFFER_SIZE;

int Writer(void *data);
int Reader(void *data);
int WriterBuffer(void *data);

int
main(int argc, char *argv[])
{
    srand(100005);
    
    pthread_t bufferedWriterThread;
    pthread_t writerThreads[WRITERS_COUNT];
    pthread_t readerThreads[READERS_COUNT];

    int rc;

    // Create Buffer Writer thread
    rc = pthread_create(
                &bufferedWriterThread,
                NULL,
                (void *) WriterBuffer,
                NULL);

    if (rc != 0) {
            errno = rc;
            err(EXIT_FAILURE, "Couldn't create the writer thread");
    }

    // Create Writer threads
    for (int ii = 0; ii < WRITERS_COUNT; ii++) {
        usleep(GetRandomTime(1000));
        int *threadId = malloc(sizeof(int));
        *threadId = ii;
        rc = pthread_create(
                writerThreads + ii,  // thread identifier
                NULL,           // thread attributes
                (void *) Writer, // thread function
                (void *) threadId);  // thread function argument
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
                (void *) Reader,     // thread function
                (void *) threadId);  // thread function argument

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


    pthread_join(bufferedWriterThread, NULL);

    return (0);
}

int
Reader(void *data)
{
    int threadId = *(int*) data;
    int result;
    
    for (int ii = 0; ii < READER_TURNS; ii++) {
        MUTEX_LOCK(result, readersCountUpdate);
        readersCount++;
        if (readersCount == 1) {
            MUTEX_LOCK(result, mutex);
        }
        MUTEX_UNLOCK(result, readersCountUpdate);

        printf("(R) Reader %d started reading... reading now %d\n", threadId, readersCount);
	    fflush(stdout);

        // Read, read, read
        usleep(GetRandomTime(200));

        MUTEX_LOCK(result, readersCountUpdate);
        readersCount--;

	    printf("(R) Reader %d finished, reading now %d\n", threadId, readersCount);
        fflush(stdout);
        if (readersCount == 0) {
            MUTEX_UNLOCK(result, mutex);
        }
            
	    // Release ownership of the mutex object.
	    MUTEX_UNLOCK(result, readersCountUpdate); 
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
        MUTEX_LOCK(result, writerCondLock);

        // Wait for space in buffer
        while (bufferSpace == 0) {
            printf("(W) Writer %d started waiting for space\n", threadId);
            result = pthread_cond_wait(&condWriter, &writerCondLock);
            if (result) {
                err(EXIT_FAILURE, "Condition wait error. Line[%d]", __LINE__);
            }
        }

        // Reserve buffer space for a single write
        bufferSpace--;

        printf("(W) Writer %d started writing to buff...\n", threadId);
        printf("(W) Buffer space left after reserving space: %d\n", bufferSpace);
        fflush(stdout);

        // Write, write, write
        usleep(GetRandomTime(800));

        printf("(W) Writer %d finished writing to buff, buffer space left: %d\n", threadId, bufferSpace);
        fflush(stdout);

        MUTEX_UNLOCK(result, writerCondLock);

    	// Think, think, think, think
        usleep(GetRandomTime(1000));
    }

    return (0);
}

int
WriterBuffer(void *data)
{
    int result;
    int writersToHandle = WRITERS_COUNT * WRITER_TURNS;

    while  (writersToHandle) {
        MUTEX_LOCK(result, writerCondLock);
        MUTEX_LOCK(result, mutex);

	    printf("(BW) BufferedWriter started emptying buffer...\n");
        fflush(stdout);

        // Write, write, write
        usleep(GetRandomTime((BUFFER_SIZE - bufferSpace + 1) * 500));
        writersToHandle -= (BUFFER_SIZE - bufferSpace);

        bufferSpace = BUFFER_SIZE;

        


        printf("(BW) BufferedWriter finished, buffer space: %d\n", bufferSpace);
        fflush(stdout);

        MUTEX_UNLOCK(result, mutex);

        result = pthread_cond_signal(&condWriter);
        if (result) {
            errno = result;
            err(EXIT_FAILURE, "Condition signaling error. Line[%d]", __LINE__);
        }

        MUTEX_UNLOCK(result, writerCondLock);


        usleep(BUFFER_EMPTY_INTERVAL);
    }

    return (0);
}
