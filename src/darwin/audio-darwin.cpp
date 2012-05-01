/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include <stdio.h>

#include "audio-darwin.h"
#include <pthread.h>

#define AudioQueueBuffer_NUM_BUFFERS 5
#define AudioQueueBuffer_SIZE 200

struct WatchCbQueueTag {
    void (*callback)(uint32_t);
    WatchCbQueueTag *pNext;
};

typedef struct
{
    AudioQueueRef pQueue;
    AudioQueueBufferRef pBuffers[AudioQueueBuffer_NUM_BUFFERS];
    CFMutableSetRef pBuffersSet;

    uint8_t *pAudioData;
    uint32_t audioLength;
    uint32_t currentPosition;

    uint32_t buffersPending;

    bool started;

    WatchCbQueueTag *pFirstWatchCb, *pLastWatchCb;

    pthread_mutex_t *pMutex;
} HnAudio_impl_Darwin;

static void enqueue_watch(HnAudio_impl_Darwin *pImpl,
        void (*callback)(uint32_t))
{
    WatchCbQueueTag *pNew = (WatchCbQueueTag *)calloc(1,
            sizeof(WatchCbQueueTag));

    pNew->callback = callback;
    pNew->pNext = NULL;

    WatchCbQueueTag *pPrev = pImpl->pLastWatchCb;

    if (pPrev != NULL)
    {
        pPrev->pNext = pNew;
    }

    if (pImpl->pFirstWatchCb == NULL)
    {
        pImpl->pFirstWatchCb = pNew;
    }

    pImpl->pLastWatchCb = pNew;
}

static void signal_pending_all(HnAudio_impl_Darwin *pImpl)
{
    for (WatchCbQueueTag *pTag = pImpl->pFirstWatchCb; pTag != NULL;
            pTag = pTag->pNext) {
        pTag->callback(pImpl->buffersPending);
    }
}

static void reset_audio(HnAudio_impl_Darwin *pImpl)
{
    pthread_mutex_lock(pImpl->pMutex);

    if (pImpl->pAudioData != NULL) {
        free(pImpl->pAudioData);
        pImpl->pAudioData = NULL;
    }

    pImpl->audioLength = 0;
    pImpl->currentPosition = 0;

    pthread_mutex_unlock(pImpl->pMutex);
}

static void buffer_complete_callback(void *inUserData, 
        AudioQueueRef inAQ, AudioQueueBufferRef inCompleteAQBuffer)
{
    HnAudio_impl_Darwin *pImpl = (HnAudio_impl_Darwin *)inUserData;
    uint8_t *pCoreAudioBuffer = (uint8_t *) inCompleteAQBuffer->mAudioData;

    pthread_mutex_lock(pImpl->pMutex);
    if (pImpl->currentPosition < pImpl->audioLength)
    {
        if (!CFSetContainsValue(pImpl->pBuffersSet, inCompleteAQBuffer)) {
            pImpl->buffersPending++;
            CFSetAddValue(pImpl->pBuffersSet, inCompleteAQBuffer);
        }

        uint32_t amountLeft = pImpl->audioLength -
            pImpl->currentPosition;

        uint32_t bytes;
        if (amountLeft <= AudioQueueBuffer_SIZE)
        {
            bytes = amountLeft;
        }
        else
        {
            bytes = AudioQueueBuffer_SIZE;
        }
        memcpy(pCoreAudioBuffer, &pImpl->pAudioData[pImpl->currentPosition],
                bytes);
        pImpl->currentPosition += bytes;
        inCompleteAQBuffer->mAudioDataByteSize = bytes;
        pthread_mutex_unlock(pImpl->pMutex);

        AudioQueueEnqueueBuffer(inAQ, inCompleteAQBuffer, 0, NULL);
    }
    else
    {
        if (CFSetContainsValue(pImpl->pBuffersSet, inCompleteAQBuffer)) {
            pImpl->buffersPending--;
            CFSetRemoveValue(pImpl->pBuffersSet, inCompleteAQBuffer);
        }
    }

    pthread_mutex_unlock(pImpl->pMutex);
    signal_pending_all(pImpl);
}

static void write_audio_to_buffer(HnAudio_impl_Darwin *pImpl,
        AudioQueueRef inAQ, AudioQueueBufferRef pBuffer) {
    if (CFSetContainsValue(pImpl->pBuffersSet, pBuffer))
    {
        return;
    }

    buffer_complete_callback(pImpl, inAQ, pBuffer);
}

void hn_darwin_audio_open(HnAudio *pAudio)
{
    HnAudio_impl_Darwin *pImpl = (HnAudio_impl_Darwin *)calloc(1,
            sizeof(HnAudio_impl_Darwin));

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

    pAudio->pImpl = pImpl;
    pAudio->close = hn_darwin_audio_close;
    pAudio->write = hn_darwin_audio_write;
    pAudio->watch = hn_darwin_audio_watch;

    pImpl->pBuffersSet = CFSetCreateMutable(NULL, 0, NULL);

    for (int i = 0; i < AudioQueueBuffer_NUM_BUFFERS; i++) {
        AudioQueueAllocateBuffer(pImpl->pQueue, AudioQueueBuffer_SIZE,
                &pImpl->pBuffers[i]);
    }

    pImpl->pMutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));

    pthread_mutex_init(pImpl->pMutex, NULL);

}

void hn_darwin_audio_watch(HnAudio *pAudio, void (*callback)(uint32_t))
{
    HnAudio_impl_Darwin *pImpl = (HnAudio_impl_Darwin *)pAudio->pImpl;
    enqueue_watch(pImpl, callback);
}

static void set_audio(HnAudio_impl_Darwin *pImpl, uint8_t *pData, uint32_t len)
{
    reset_audio(pImpl);

    pthread_mutex_lock(pImpl->pMutex);
    pImpl->pAudioData = pData;
    pImpl->audioLength = len;
    pthread_mutex_unlock(pImpl->pMutex);
}

void hn_darwin_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len)
{
    HnAudio_impl_Darwin *pImpl = (HnAudio_impl_Darwin *)pAudio->pImpl;

    set_audio(pImpl, pData, len);

    for (int i = 0; i < AudioQueueBuffer_NUM_BUFFERS; i++) {
        write_audio_to_buffer(pImpl, pImpl->pQueue, pImpl->pBuffers[i]);
    }

    if (!pImpl->started) {
        pImpl->started = true;
        AudioQueuePrime(pImpl->pQueue, 0, NULL);
        AudioQueueStart(pImpl->pQueue, NULL);
    }

    signal_pending_all(pImpl);
}

void hn_darwin_audio_close(HnAudio *pAudio)
{
    HnAudio_impl_Darwin *pImpl = (HnAudio_impl_Darwin *)pAudio->pImpl;

    reset_audio(pImpl);

    CFRelease(pImpl->pBuffersSet);
    AudioQueueDispose(pImpl->pQueue, true);

    pthread_mutex_destroy(pImpl->pMutex);

    free(pImpl->pMutex);
    free(pImpl);
    free(pAudio);
}
