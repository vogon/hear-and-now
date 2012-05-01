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
    CFMutableSetRef pBuffersSet;
    CFMutableSetRef pPlayingBuffersSet;

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
        if (!CFSetContainsValue(pImpl->pPlayingBuffersSet, inCompleteAQBuffer))
        {
            pImpl->buffersPending++;
            CFSetAddValue(pImpl->pPlayingBuffersSet, inCompleteAQBuffer);
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
        if (CFSetContainsValue(pImpl->pPlayingBuffersSet, inCompleteAQBuffer))
        {
            pImpl->buffersPending--;
            CFSetRemoveValue(pImpl->pPlayingBuffersSet, inCompleteAQBuffer);
        }

        hn_mutex_unlock(pImpl->pMutex);
    }

    signal_pending_all(pImpl);
}

static AudioStreamBasicDescription *convertFormat(HnAudioFormat *pFormat) {
    AudioStreamBasicDescription *result = (AudioStreamBasicDescription *)
        calloc(1, sizeof(AudioStreamBasicDescription));

    result->mSampleRate = pFormat->samplesPerSecond;
    result->mFormatID = kAudioFormatLinearPCM;
    result->mFramesPerPacket = 1;
    result->mChannelsPerFrame = pFormat->numberOfChannels;
    result->mBytesPerPacket = (pFormat->sampleResolution / 8) * result->mChannelsPerFrame;
    result->mBytesPerFrame = result->mFramesPerPacket * result->mBytesPerPacket;
    result->mBitsPerChannel = pFormat->sampleResolution;
    result->mReserved = 0;
    result->mFormatFlags = 0;

    return result;
}

HnAudio *hn_darwin_audio_open(HnAudioFormat *pFormat)
{
    HnAudio_Darwin *pImpl = (HnAudio_Darwin *)malloc(sizeof(HnAudio_Darwin));

    AudioStreamBasicDescription *pCoreAudioFormat = convertFormat(pFormat);

    AudioQueueNewOutput(pCoreAudioFormat, buffer_complete_callback, pImpl,
            NULL, kCFRunLoopCommonModes, 0, &pImpl->pQueue);

    free(pCoreAudioFormat);

    pImpl->pBuffersSet = CFSetCreateMutable(NULL, 0, NULL);

    for (int i = 0; i < AudioQueueBuffer_NUM_BUFFERS; i++)
    {
        AudioQueueBufferRef pBuffer;
        AudioQueueAllocateBuffer(pImpl->pQueue, AudioQueueBuffer_SIZE, &pBuffer);
        CFSetAddValue(pImpl->pBuffersSet, pBuffer);
    }

    pImpl->pPlayingBuffersSet = CFSetCreateMutable(NULL, 0, NULL);

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

static void write_audio_to_buffer(const void *value, void *context) {
    HnAudio_Darwin *pImpl = (HnAudio_Darwin *) context;
    AudioQueueBufferRef pBuffer = (AudioQueueBufferRef) value;

    if (CFSetContainsValue(pImpl->pPlayingBuffersSet, pBuffer))
    {
        return;
    }

    buffer_complete_callback(pImpl, pImpl->pQueue, pBuffer);
}

void hn_darwin_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len)
{
    HnAudio_Darwin *pImpl = (HnAudio_Darwin *)pAudio;

    set_audio(pImpl, pData, len);

    CFSetApplyFunction(pImpl->pBuffersSet, write_audio_to_buffer, pImpl);

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
    CFRelease(pImpl->pPlayingBuffersSet);

    AudioQueueDispose(pImpl->pQueue, true);

    hn_mutex_unlock(pImpl->pMutex);

    hn_mutex_destroy(pImpl->pMutex);

    free(pAudio);
}
