/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "audio-darwin.h"

#define AudioQueueBuffer_NUM_BUFFERS 10
#define AudioQueueBuffer_SIZE 2048

typedef struct WatchCbQueueTag
{
    void (*callback)(void *, uint32_t);
    void *context;
    struct WatchCbQueueTag *pNext;
} WatchCbQueueTag;

typedef struct
{
    const void *pVtbl;

    AudioQueueRef pQueue;
    AudioQueueBufferRef pBuffers[AudioQueueBuffer_NUM_BUFFERS];
    CFMutableSetRef pBuffersSet;

    uint8_t *pAudioData;
    uint32_t audioLength;
    uint32_t currentPosition;

    uint32_t buffersPending;

    bool started;

    WatchCbQueueTag *pFirstWatchCb, *pLastWatchCb;

    HnMutex *pMutex;
} HnAudio_Darwin;

const HnAudio_vtbl _HnAudio_Darwin_vtbl =
{
    hn_darwin_audio_watch,
    hn_darwin_audio_write,
    hn_darwin_audio_samples_pending,
    hn_darwin_audio_close,
};

static void enqueue_watch(HnAudio_Darwin *pAudioImpl,
        void (*callback)(void *, uint32_t), void *context)
{
    WatchCbQueueTag *pNew = (WatchCbQueueTag *)malloc(sizeof(WatchCbQueueTag));

    pNew->callback = callback;
    pNew->context = context;
    pNew->pNext = NULL;

    WatchCbQueueTag *pPrev = pAudioImpl->pLastWatchCb;

    if (pPrev != NULL)
    {
        pPrev->pNext = pNew;
    }

    if (pAudioImpl->pFirstWatchCb == NULL)
    {
        pAudioImpl->pFirstWatchCb = pNew;
    }

    pAudioImpl->pLastWatchCb = pNew;    
}

static void signal_pending_all(HnAudio_Darwin *pImpl)
{
    for (WatchCbQueueTag *pTag = pImpl->pFirstWatchCb; pTag != NULL;
            pTag = pTag->pNext) {
        pTag->callback(pTag->context, pImpl->buffersPending);
    }
}

static void buffer_complete_callback(void *inUserData, 
        AudioQueueRef inAQ, AudioQueueBufferRef inCompleteAQBuffer)
{
    HnAudio_Darwin *pImpl = (HnAudio_Darwin *)inUserData;
    uint8_t *pAudioData = (uint8_t *) inCompleteAQBuffer->mAudioData;

    if (pImpl->currentPosition < pImpl->audioLength)
    {
        if (!CFSetContainsValue(pImpl->pBuffersSet, inCompleteAQBuffer)) {
            pImpl->buffersPending++;
            CFSetAddValue(pImpl->pBuffersSet, inCompleteAQBuffer);
        }

        uint32_t amountLeft = pImpl->audioLength -
            pImpl->currentPosition;

        uint32_t numBytes;
        if (amountLeft <= AudioQueueBuffer_SIZE)
        {
            numBytes = amountLeft;
        }
        else
        {
            numBytes = AudioQueueBuffer_SIZE;
        }

        memcpy(pAudioData, &pImpl->pAudioData[pImpl->currentPosition],
                numBytes);

        pImpl->currentPosition += numBytes;

        hn_mutex_unlock(pImpl->pMutex);

        inCompleteAQBuffer->mAudioDataByteSize = numBytes;
        AudioQueueEnqueueBuffer(inAQ, inCompleteAQBuffer, 0, NULL);
    }
    else
    {
        if (CFSetContainsValue(pImpl->pBuffersSet, inCompleteAQBuffer)) {
            pImpl->buffersPending--;
            CFSetRemoveValue(pImpl->pBuffersSet, inCompleteAQBuffer);
        }

        hn_mutex_unlock(pImpl->pMutex);
    }

    signal_pending_all(pImpl);
}

HnAudio *hn_darwin_audio_open(HnAudioFormat *pFormat)
{
    HnAudio_Darwin *pImpl = (HnAudio_Darwin *)calloc(1,
            sizeof(HnAudio_Darwin));

    AudioStreamBasicDescription format;
    format.mSampleRate = 44100.0;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFramesPerPacket = 1;
    format.mChannelsPerFrame = 1;
    format.mBytesPerFrame = 1;
    format.mBytesPerPacket = 1;
    format.mBitsPerChannel = 8;
    format.mReserved = 0;
    format.mFormatFlags = 0;

    AudioQueueNewOutput(&format, buffer_complete_callback, pImpl, NULL,
            kCFRunLoopCommonModes, 0, &pImpl->pQueue);

    pImpl->pBuffersSet = CFSetCreateMutable(NULL, 0, NULL);

    for (int i = 0; i < AudioQueueBuffer_NUM_BUFFERS; i++)
    {
        AudioQueueAllocateBuffer(pImpl->pQueue, AudioQueueBuffer_SIZE,
                &pImpl->pBuffers[i]);
    }

    pImpl->pMutex = hn_mutex_create();
    pImpl->pVtbl = &_HnAudio_Darwin_vtbl;

    return (HnAudio *)pImpl;
}

void hn_darwin_audio_watch(HnAudio *pAudio, void (*callback)(void *, uint32_t),
        void *context)
{
    enqueue_watch((HnAudio_Darwin *)pAudio, callback, context);
}

static void reset_audio(HnAudio_Darwin *pImpl)
{
    hn_mutex_lock(pImpl->pMutex);

    if (pImpl->pAudioData != NULL) {
        free(pImpl->pAudioData);
        pImpl->pAudioData = NULL;
        pImpl->audioLength = 0;
        pImpl->currentPosition = 0;
    }

    hn_mutex_unlock(pImpl->pMutex);
}

static void set_audio(HnAudio_Darwin *pImpl, uint8_t *pData, uint32_t len)
{
    reset_audio(pImpl);

    hn_mutex_lock(pImpl->pMutex);

    pImpl->pAudioData = pData;
    pImpl->audioLength = len;
    pImpl->currentPosition = 0;

    hn_mutex_unlock(pImpl->pMutex);
}


static void write_audio_to_buffer(HnAudio_Darwin *pImpl, AudioQueueRef inAQ, 
        AudioQueueBufferRef pBuffer)
{
    if (CFSetContainsValue(pImpl->pBuffersSet, pBuffer))
    {
        return;
    }

    buffer_complete_callback(pImpl, inAQ, pBuffer);
}

void hn_darwin_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len)
{
    HnAudio_Darwin *pImpl = (HnAudio_Darwin *)pAudio;

    set_audio(pImpl, pData, len);

    for (int i = 0; i < AudioQueueBuffer_NUM_BUFFERS; i++)
    {
        write_audio_to_buffer(pImpl, pImpl->pQueue, pImpl->pBuffers[i]);
    }

    hn_mutex_lock(pImpl->pMutex);
    if (!pImpl->started)
    {
        pImpl->started = true;

        hn_mutex_unlock(pImpl->pMutex);

        AudioQueuePrime(pImpl->pQueue, 0, NULL);
        AudioQueueStart(pImpl->pQueue, NULL);
    }
    else
    {
        hn_mutex_unlock(pImpl->pMutex);
    }
}

uint32_t hn_darwin_audio_samples_pending(HnAudio *pAudio)
{
    HnAudio_Darwin *pImpl = (HnAudio_Darwin *)pAudio;
    return pImpl->buffersPending;
}

void hn_darwin_audio_close(HnAudio *pAudio)
{
    HnAudio_Darwin *pImpl = (HnAudio_Darwin *)pAudio;

    reset_audio(pImpl);

    hn_mutex_lock(pImpl->pMutex);

    CFRelease(pImpl->pBuffersSet);
    AudioQueueDispose(pImpl->pQueue, true);

    hn_mutex_unlock(pImpl->pMutex);

    hn_mutex_destroy(pImpl->pMutex);

    free(pAudio);
}
