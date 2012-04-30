/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#ifndef _HN_LOCKS_H
#define _HN_LOCKS_H

#ifdef __cplusplus
extern "C" {
#endif

struct HnMutex;
typedef struct HnMutex HnMutex;

struct HnConditionVariable;
typedef struct HnConditionVariable HnConditionVariable;

HnMutex *hn_mutex_create();

void hn_mutex_lock(HnMutex *pMutex);
void hn_mutex_unlock(HnMutex *pMutex);

void hn_mutex_destroy(HnMutex *pMutex);

HnConditionVariable *hn_cv_create();

void hn_cv_sleep(HnConditionVariable *pCv,
                 HnMutex *pLock);

void hn_cv_wake(HnConditionVariable *pCv);

#ifdef __cplusplus
}
#endif

#endif /* _HN_LOCKS_H */