#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#define DEBUG

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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t writeHappenedCond = PTHREAD_COND_INITIALIZER;
sem_t semaphoreCritic;

volatile int writeHappened = 0;
volatile int whoWrote = -1;

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

    rc = sem_init(&semaphoreCritic, 0, 1);

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

    pthread_join(criticThread, NULL);

    return (0);
}

int
Reader(void *data)
{
    int threadId = *(int*) data;
    int result;
    
    for (int ii = 0; ii < READER_TURNS; ii++) {
        dprint("(R) Reader %d trying to take mutex\n", threadId);
        MUTEX_LOCK(result, mutex);
        dprint("(R) Reader %d obtained mutex\n", threadId);

        print("(R) Reader %d started reading\n", threadId);

        // Read, read, read

	    usleep(GetRandomTime(200));
	    print("(R) Reader %d finished reading\n", threadId);

	    // Release ownership of the mutex object.
        MUTEX_UNLOCK(result, mutex);
        dprint("(R) Reader %d unlocked mutex\n", threadId);

	    
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
        dprint("(W) Writer %d waiting for critic semaphore\n", threadId);
        sem_wait(&semaphoreCritic);
        dprint("(W) Writer %d trying to take mutex\n", threadId);
        MUTEX_LOCK(result, mutex);
        dprint("(W) Writer %d obtained mutex\n", threadId);

	    print("(W) Writer %d started writing...\n", threadId);

        // Write, write, write
    	usleep(GetRandomTime(800));

    	print("(W) Writer %d finished writing\n", threadId);

        // Inform critic, that write happened
        write_happened();
        whoWrote = threadId;
        result = pthread_cond_signal(&writeHappenedCond);
        if ( result ) {
            errno = result;
            err(EXIT_FAILURE,
                "Condition signaling error. Line [%d]",
                __LINE__);
        }

	    // Release ownership of the mutex object.
        MUTEX_UNLOCK(result, mutex);
        dprint("(W) Writer %d unlocked mutex\n", threadId);
    	    
        // Think, think, think, think
	    usleep(GetRandomTime(1000));
    }

    free(data);

    return (0);
}

int
Critic(void *data)
{
    int numOfActions = WRITERS_COUNT * WRITER_TURNS;    
    int result;

    while (numOfActions) {
        dprint("(C) Critic trying to lock mutex\n");
        MUTEX_LOCK(result, mutex);
        dprint("(C) Critic obtained mutex\n");

        while ( !writeHappened ) {
            dprint("(C) Critic waiting for somebody to write\n");
            result = pthread_cond_wait(&writeHappenedCond, &mutex);
            if ( result ) {
                errno = result;
                err(EXIT_FAILURE, "Waiting for condition variable error.");
            }
        }

        print("(C) Critic started criticising work of Writer %d\n", whoWrote);

        /* Criticise, criticise, criticise */
        usleep(GetRandomTime(500));

        print("(C) Critic finished criticising\n");

        /* Inform writers, that critic finished */
        write_handled();
        numOfActions--;
        
        MUTEX_UNLOCK(result, mutex);
        dprint("(C) Critic unlocked mutex\n");
        sem_post(&semaphoreCritic);
        dprint("(C) Critic posted critic semaphore - now Writers can write again.\n");
    }

    return (0);
}
