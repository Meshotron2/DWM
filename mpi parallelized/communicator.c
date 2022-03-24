#include "communicator.h"

static void* ThreadLoop(void* args);

static FaceBuffer* faces;
static int faceCount;

static MPI_Request requests[MAX_REQUESTS];
static pthread_mutex_t access;

static pthread_cond_t awake;
static bool isAwake;

static pthread_cond_t done;
static bool isDone;

static pthread_cond_t waiting;
static bool isWaiting;

static pthread_cond_t initialized;
static bool isInitialized;

static int world_rank, world_size;
static int* argc;
static bool isInitialized;
static char*** argv;

static bool alive;

void CommunicatorInit(int* rank, int* size, int* ac, char*** av)
{
    isAwake = false;
    isDone = false;
    isWaiting = false;
    alive = true;
    isInitialized = false;

    pthread_mutex_init(&access, NULL);
    pthread_cond_init(&awake, NULL);
    pthread_cond_init(&done, NULL);
    pthread_cond_init(&waiting, NULL);
    pthread_cond_init(&initialized, NULL);

    pthread_create(&communicatorThread, NULL, ThreadLoop, NULL);

    pthread_mutex_lock(&access);
    while(!isInitialized)
    {
        pthread_cond_wait(&initialized, &access);
    }

    pthread_cond_destroy(&initialized);

    *rank = world_rank;
    *size = world_size;
    argc = ac;
    argv = av;
    pthread_mutex_unlock(&access);
}

void CommunicatorSetData(FaceBuffer* f, int count)
{
    faces = f;
    faceCount = count;
}

void CommunicatorDestroy()
{
    // pthread_cancel(communicatorThread);
    pthread_mutex_lock(&access);
    alive = false;
    isAwake = true;
    pthread_cond_broadcast(&awake);
    pthread_mutex_unlock(&access);
    pthread_join(communicatorThread, NULL);

    pthread_mutex_destroy(&access);
    pthread_cond_destroy(&awake);
    pthread_cond_destroy(&done);
    pthread_cond_destroy(&waiting);
}

void Send()
{
    pthread_mutex_lock(&access);

    isAwake = true;
    pthread_cond_broadcast(&awake);

    pthread_mutex_unlock(&access);
}

void Wait()
{
    pthread_mutex_lock(&access);

    isWaiting = true;
    pthread_cond_broadcast(&waiting);
    
    while(!isDone)
    {
        pthread_cond_wait(&done, &access);
    }

    isDone = false;
    isWaiting = false;

    pthread_mutex_unlock(&access);
}

static void* ThreadLoop(void* args)
{
    int provided;
    MPI_Init_thread(argc, argv, MPI_THREAD_FUNNELED, &provided);
    if(provided < MPI_THREAD_FUNNELED)
    {
        printf("Failed to get MPI_THREAD_FUNNELED\n");
        exit(EXIT_FAILURE);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    pthread_mutex_lock(&access);
    isInitialized = true;
    pthread_cond_broadcast(&initialized);
    pthread_mutex_unlock(&access);

    while(true)
    {        
        pthread_mutex_lock(&access);
        while (!isAwake)
        {
            pthread_cond_wait(&awake, &access);
        }

        pthread_mutex_unlock(&access);

        if(!alive) break;

        for(int i = 0, j = MAX_REQUESTS / 2; i < faceCount; i++, j++)
        {
            FaceBuffer* cFace = &faces[i];
            MPI_Irecv(cFace->inData, cFace->size, MPI_FLOAT, cFace->neighbour, cFace->face, MPI_COMM_WORLD, &requests[i]);
            MPI_Isend(cFace->outData, cFace->size, MPI_FLOAT, cFace->neighbour, cFace->neighbourFace, MPI_COMM_WORLD, &requests[j]);
        }

        MPI_Waitall(faceCount, requests, MPI_STATUS_IGNORE);

        pthread_mutex_lock(&access);
        while(!isWaiting)
        {
            pthread_cond_wait(&waiting, &access);
        }

        isDone = true;
        isAwake = false;

        pthread_cond_broadcast(&done);
        pthread_mutex_unlock(&access);
    }

    MPI_Finalize();

    return NULL;
}