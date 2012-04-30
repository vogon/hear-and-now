/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "locks.h"

#include <windows.h>

struct HnMutex
{
    CRITICAL_SECTION cs;
};

struct HnConditionVariable
{
    CONDITION_VARIABLE cv;
};

HnMutex *hn_mutex_create()
{
    HnMutex *result = (HnMutex *)malloc(sizeof(HnMutex));
    InitializeCriticalSection(&(result->cs));

    return result;
}

void hn_mutex_lock(HnMutex *pMutex)
{
    EnterCriticalSection(&(pMutex->cs));
}

void hn_mutex_unlock(HnMutex *pMutex)
{
    LeaveCriticalSection(&(pMutex->cs));
}

void hn_mutex_destroy(HnMutex *pMutex)
{
    DeleteCriticalSection(&(pMutex->cs));
    free(pMutex);
}

HnConditionVariable *hn_cv_create()
{
    HnConditionVariable *result = 
        (HnConditionVariable *)malloc(sizeof(HnConditionVariable));
    InitializeConditionVariable(&(result->cv));

    return result;
}

void hn_cv_sleep(HnConditionVariable *pCv,
                 HnMutex *pLock)
{
    SleepConditionVariableCS(&(pCv->cv), &(pLock->cs), 0);
}

void hn_cv_wake(HnConditionVariable *pCv)
{
    WakeConditionVariable(&(pCv->cv));
}