/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "locks.h"

#include <pthread.h>
#include <stdlib.h>

struct HnMutex
{
    pthread_mutex_t mutex;
};

struct HnConditionVariable
{
    pthread_cond_t cv;
};

HnMutex *hn_mutex_create()
{
    HnMutex *result = (HnMutex *)malloc(sizeof(HnMutex));

    pthread_mutex_init(&(result->mutex), NULL);

    return result;
}

void hn_mutex_lock(HnMutex *pMutex)
{
    pthread_mutex_lock(&(pMutex->mutex));
}

void hn_mutex_unlock(HnMutex *pMutex)
{
    pthread_mutex_unlock(&(pMutex->mutex));
}

void hn_mutex_destroy(HnMutex *pMutex)
{
    pthread_mutex_destroy(&(pMutex->mutex));
    free(pMutex);
}

HnConditionVariable *hn_cv_create()
{
    HnConditionVariable *result = 
        (HnConditionVariable *)malloc(sizeof(HnConditionVariable));

    pthread_cond_init(&(result->cv), NULL);

    return result;
}

void hn_cv_sleep(HnConditionVariable *pCv,
                 HnMutex *pLock)
{
    pthread_cond_wait(&(pCv->cv), &(pLock->mutex));
}

void hn_cv_wake(HnConditionVariable *pCv)
{
    pthread_cond_signal(&(pCv->cv));
}
