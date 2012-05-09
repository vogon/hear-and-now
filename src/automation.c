/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "hn.h"
#include "automation.h"

#include <stdlib.h>

typedef struct HnCmdQueueTag
{
    HnCmd *pCmd;
    struct HnCmdQueueTag *pNext;
} HnCmdQueueTag;

struct HnCmdQueue
{
    HnCmdQueueTag *pFirst, *pLast;
};

HnCmdQueue *hn_cmd_queue_create()
{
    HnCmdQueue *pQueue = (HnCmdQueue *)calloc(1, sizeof(HnCmdQueue));

    return pQueue;
}

void hn_cmd_queue_send(HnCmdQueue *pQueue, HnCmd *pCmd)
{
    HnCmdQueueTag *pNew = (HnCmdQueueTag *)malloc(sizeof(HnCmdQueueTag));
    pNew->pCmd = pCmd;
    pNew->pNext = NULL;

    HnCmdQueueTag *pPrev = pQueue->pLast;

    if (pPrev != NULL)
    {
        pPrev->pNext = pNew;
    }

    if (pQueue->pFirst == NULL)
    {
        pQueue->pFirst = pNew;
    }

    pQueue->pLast = pNew;
}

HnCmd *hn_cmd_queue_pop(HnCmdQueue *pQueue)
{
    HnCmdQueueTag *pTag = pQueue->pFirst;

    if (NULL == pTag)
    {
        return NULL;
    }
    else
    {
        if (NULL == pTag->pNext)
        {
            pQueue->pLast = NULL;
        }

        pQueue->pFirst = pTag->pNext;

        HnCmd *pCmd = pTag->pCmd;

        free(pTag);
        return pCmd;
    }
}