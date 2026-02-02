#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "ao.h"

static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine = NULL;
static bool initialized = false;

struct ao_device {
    SLObjectItf outputMixObject;
    SLObjectItf playerObject;
    SLPlayItf playerPlay;
    SLAndroidSimpleBufferQueueItf bufferQueue;
    int driver_type;
    ao_sample_format format;
    char *filename;
    FILE *file;
    char *buffer;
    size_t buffer_size;
    size_t buffer_pos;
};

static ao_option *global_options = NULL;

void ao_initialize(void)
{
    if (initialized) return;

    SLresult result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        errno = AO_EFAIL;
        return;
    }

    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        errno = AO_EFAIL;
        return;
    }

    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (result != SL_RESULT_SUCCESS) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        errno = AO_EFAIL;
        return;
    }

    initialized = true;
    errno = 0;
}

void ao_shutdown(void)
{
    if (!initialized) return;

    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    ao_free_options(global_options);
    global_options = NULL;
    initialized = false;
}

int ao_append_global_option(const char *key, const char *value)
{
    (void)key;
    (void)value;
    return 1;
}

int ao_append_option(ao_option **options, const char *key, const char *value)
{
    ao_option *opt = (ao_option *)malloc(sizeof(ao_option));
    if (!opt) return 0;

    opt->key = strdup(key);
    opt->value = strdup(value);
    opt->next = *options;
    *options = opt;

    return 1;
}

void ao_free_options(ao_option *opt)
{
    while (opt) {
        ao_option *next = opt->next;
        free(opt->key);
        free(opt->value);
        free(opt);
        opt = next;
    }
}

static int bits_to_pcmconf(int bits)
{
    switch (bits) {
        case 8: return SL_PCMSAMPLEFORMAT_FIXED_8;
        case 16: return SL_PCMSAMPLEFORMAT_FIXED_16;
        case 24: return SL_PCMSAMPLEFORMAT_FIXED_24;
        case 32: return SL_PCMSAMPLEFORMAT_FIXED_32;
        default: return SL_PCMSAMPLEFORMAT_FIXED_16;
    }
}

static int byte_format_to_endian(int byte_format)
{
    union {
        uint32_t i;
        char c[4];
    } test = { 0x01020304 };
    bool is_big = test.c[0] == 1;

    switch (byte_format) {
        case AO_FMT_LITTLE: return SL_BYTEORDER_LITTLEENDIAN;
        case AO_FMT_BIG: return SL_BYTEORDER_BIGENDIAN;
        case AO_FMT_NATIVE:
        default: return is_big ? SL_BYTEORDER_BIGENDIAN : SL_BYTEORDER_LITTLEENDIAN;
    }
}

static void convert_samples(ao_sample_format *fmt, const char *input,
                            char *output, size_t samples)
{
    size_t i;
    int16_t *in16 = (int16_t *)input;
    int16_t *out16 = (int16_t *)output;

    for (i = 0; i < samples; i++) {
        int16_t sample = in16[i];
        switch (fmt->byte_format) {
            case AO_FMT_LITTLE:
                output[i * 2] = (char)(sample & 0xFF);
                output[i * 2 + 1] = (char)(sample >> 8);
                break;
            case AO_FMT_BIG:
                output[i * 2] = (char)(sample >> 8);
                output[i * 2 + 1] = (char)(sample & 0xFF);
                break;
            case AO_FMT_NATIVE:
            default:
                out16[i] = sample;
                break;
        }
    }
}

static void bufferQueueCallback(SLAndroidSimpleBufferQueueItf bufferQueue,
                                void *context)
{
    (void)bufferQueue;
    (void)context;
}

ao_device *ao_open_live(int driver_id, ao_sample_format *format, ao_option *option)
{
    (void)driver_id;
    (void)option;

    if (!engineEngine) {
        errno = AO_EFAIL;
        return NULL;
    }

    ao_device *dev = (ao_device *)malloc(sizeof(ao_device));
    if (!dev) {
        errno = AO_EFAIL;
        return NULL;
    }

    memset(dev, 0, sizeof(ao_device));
    dev->driver_type = AO_TYPE_LIVE;
    dev->format = *format;
    dev->buffer_size = format->rate * format->channels * (format->bits / 8);
    dev->buffer = (char *)malloc(dev->buffer_size);
    if (!dev->buffer) {
        free(dev);
        errno = AO_EFAIL;
        return NULL;
    }

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        2
    };

    int pcm_format = bits_to_pcmconf(format->bits);
    int endianness = byte_format_to_endian(format->byte_format);

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        (SLuint32)format->channels,
        (SLuint32)(format->rate * 1000),
        (SLuint32)pcm_format,
        (SLuint32)pcm_format,
        (SLuint32)endianness,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT
    };

    SLDataSource audioSrc = { &loc_bufq, &format_pcm };

    SLDataLocator_OutputMix loc_outmix = {
        SL_DATALOCATOR_OUTPUTMIX,
        NULL
    };
    SLDataSink audioSnk = { &loc_outmix, NULL };

    const SLInterfaceID ids[] = { SL_IID_BUFFERQUEUE };
    const SLboolean req[] = { SL_BOOLEAN_TRUE };

    SLresult result = (*engineEngine)->CreateAudioPlayer(
        engineEngine, &dev->playerObject,
        &audioSrc, &audioSnk, 1, ids, req);

    if (result != SL_RESULT_SUCCESS) {
        free(dev->buffer);
        free(dev);
        errno = AO_EFAIL;
        return NULL;
    }

    result = (*dev->playerObject)->Realize(dev->playerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        free(dev->buffer);
        free(dev);
        errno = AO_EFAIL;
        return NULL;
    }

    result = (*dev->playerObject)->GetInterface(dev->playerObject, SL_IID_PLAY, &dev->playerPlay);
    if (result != SL_RESULT_SUCCESS) {
        (*dev->playerObject)->Destroy(dev->playerObject);
        free(dev->buffer);
        free(dev);
        errno = AO_EFAIL;
        return NULL;
    }

    result = (*dev->playerObject)->GetInterface(dev->playerObject, SL_IID_BUFFERQUEUE, &dev->bufferQueue);
    if (result != SL_RESULT_SUCCESS) {
        (*dev->playerObject)->Destroy(dev->playerObject);
        free(dev->buffer);
        free(dev);
        errno = AO_EFAIL;
        return NULL;
    }

    (*dev->bufferQueue)->RegisterCallback(dev->bufferQueue, bufferQueueCallback, NULL);

    result = (*dev->playerPlay)->SetPlayState(dev->playerPlay, SL_PLAYSTATE_PLAYING);
    if (result != SL_RESULT_SUCCESS) {
        (*dev->playerObject)->Destroy(dev->playerObject);
        free(dev->buffer);
        free(dev);
        errno = AO_EFAIL;
        return NULL;
    }

    return dev;
}

ao_device *ao_open_file(int driver_id, const char *filename, int overwrite,
                        ao_sample_format *format, ao_option *option)
{
    (void)driver_id;
    (void)option;

    if (!overwrite) {
        FILE *test = fopen(filename, "rb");
        if (test) {
            fclose(test);
            errno = AO_EFILEEXISTS;
            return NULL;
        }
    }

    ao_device *dev = (ao_device *)malloc(sizeof(ao_device));
    if (!dev) return NULL;

    memset(dev, 0, sizeof(ao_device));
    dev->driver_type = AO_TYPE_FILE;
    dev->format = *format;
    dev->filename = strdup(filename);

    dev->file = fopen(filename, "wb");
    if (!dev->file) {
        free(dev->filename);
        free(dev);
        return NULL;
    }

    unsigned char wav_header[44] = {
        'R', 'I', 'F', 'F',
        0, 0, 0, 0,
        'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ',
        16, 0, 0, 0,
        1, 0,
        (unsigned char)(format->channels),
        0,
        (unsigned char)(format->rate & 0xFF),
        (unsigned char)((format->rate >> 8) & 0xFF),
        (unsigned char)((format->rate >> 16) & 0xFF),
        (unsigned char)((format->rate >> 24) & 0xFF),
        0, 0,
        (unsigned char)((format->channels * format->bits / 8) & 0xFF),
        0,
        (unsigned char)(format->bits),
        0,
        'd', 'a', 't', 'a',
        0, 0, 0, 0
    };

    int byte_rate = format->rate * format->channels * format->bits / 8;
    wav_header[22] = 1;
    wav_header[32] = (unsigned char)(byte_rate & 0xFF);
    wav_header[33] = (unsigned char)((byte_rate >> 8) & 0xFF);
    wav_header[34] = (unsigned char)((byte_rate >> 16) & 0xFF);
    wav_header[35] = (unsigned char)((byte_rate >> 24) & 0xFF);
    wav_header[40] = (unsigned char)(format->channels * format->bits / 8);

    fwrite(wav_header, 1, 44, dev->file);

    return dev;
}

int ao_play(ao_device *device, char *samples, uint32_t num_bytes)
{
    if (!device) return 0;

    if (device->driver_type == AO_TYPE_FILE) {
        size_t written = fwrite(samples, 1, num_bytes, device->file);
        return (written == num_bytes) ? 1 : 0;
    } else if (device->driver_type == AO_TYPE_LIVE) {
        SLresult result = (*device->bufferQueue)->Enqueue(
            device->bufferQueue, samples, num_bytes);
        return (result == SL_RESULT_SUCCESS) ? 1 : 0;
    }

    return 0;
}

int ao_close(ao_device *device)
{
    if (!device) return 0;

    if (device->driver_type == AO_TYPE_FILE) {
        if (device->file) {
            fseek(device->file, 4, SEEK_SET);
            int file_size = ftell(device->file);
            fseek(device->file, 0, SEEK_END);
            file_size = ftell(device->file) - 8;
            fseek(device->file, 4, SEEK_SET);
            fwrite(&file_size, 4, 1, device->file);
            fseek(device->file, 40, SEEK_SET);
            file_size -= 36;
            fwrite(&file_size, 4, 1, device->file);
            fclose(device->file);
        }
        free(device->filename);
    } else if (device->driver_type == AO_TYPE_LIVE) {
        if (device->playerObject) {
            (*device->playerObject)->Destroy(device->playerObject);
        }
    }

    free(device->buffer);
    free(device);

    return 1;
}

int ao_driver_id(const char *driver_name)
{
    if (strcmp(driver_name, "android") == 0 ||
        strcmp(driver_name, "opensles") == 0 ||
        strcmp(driver_name, "default") == 0) {
        return 1;
    }
    return -1;
}

int ao_default_driver_id(void)
{
    return 1;
}

static ao_info live_info = {
    .type = AO_TYPE_LIVE,
    .name = "Android OpenSLES",
    .short_name = "android",
    .comment = "Android OpenSLES Audio Output",
    .priority = 100,
    .preferred_byte_format = AO_FMT_NATIVE,
    .option_count = 0,
    .options = NULL
};

static ao_info file_info = {
    .type = AO_TYPE_FILE,
    .name = "WAV File",
    .short_name = "wav",
    .comment = "WAV File Output",
    .priority = 50,
    .preferred_byte_format = AO_FMT_LITTLE,
    .option_count = 0,
    .options = NULL
};

ao_info *ao_driver_info(int driver_id)
{
    (void)driver_id;
    return &live_info;
}

ao_info **ao_driver_info_list(int *count)
{
    ao_info **list = (ao_info **)malloc(2 * sizeof(ao_info *));
    list[0] = &live_info;
    list[1] = &file_info;
    *count = 2;
    return list;
}

bool ao_is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } test = { 0x01020304 };

    return test.c[0] == 1;
}
