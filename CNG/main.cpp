#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//采用https://github.com/mackron/dr_libs/blob/master/dr_wav.h 解码
#define DR_WAV_IMPLEMENTATION

#include "dr_wav.h"
#include "cng.h"

#ifndef nullptr
#define nullptr 0
#endif

#ifndef MIN
#define MIN(A, B)        ((A) < (B) ? (A) : (B))
#endif

//写wav文件
void wavWrite_int16(char *filename, int16_t *buffer, size_t sampleRate, size_t totalSampleCount) {
    drwav_data_format format = {};
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = 1;
    format.sampleRate = (drwav_uint32) sampleRate;
    format.bitsPerSample = 16;
    drwav *pWav = drwav_open_file_write(filename, &format);
    if (pWav) {
        drwav_uint64 samplesWritten = drwav_write(pWav, totalSampleCount, buffer);
        drwav_uninit(pWav);
        if (samplesWritten != totalSampleCount) {
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
    }
}


//分割路径函数
void splitpath(const char *path, char *drv, char *dir, char *name, char *ext) {
    const char *end;
    const char *p;
    const char *s;
    if (path[0] && path[1] == ':') {
        if (drv) {
            *drv++ = *path++;
            *drv++ = *path++;
            *drv = '\0';
        }
    } else if (drv)
        *drv = '\0';
    for (end = path; *end && *end != ':';)
        end++;
    for (p = end; p > path && *--p != '\\' && *p != '/';)
        if (*p == '.') {
            end = p;
            break;
        }
    if (ext)
        for (s = end; (*ext = *s++);)
            ext++;
    for (p = end; p > path;)
        if (*--p == '\\' || *p == '/') {
            p++;
            break;
        }
    if (name) {
        for (s = p; s < end;)
            *name++ = *s++;
        *name = '\0';
    }
    if (dir) {
        for (s = path; s < p;)
            *dir++ = *s++;
        *dir = '\0';
    }
}

enum {
    kSidShortIntervalUpdate = 1,
    kSidNormalIntervalUpdate = 100,
    kSidLongIntervalUpdate = 10000
};

enum : size_t {
    kCNGNumParamsLow = 0,
    kCNGNumParamsNormal = 8,
    kCNGNumParamsHigh = WEBRTC_CNG_MAX_LPC_ORDER,
    kCNGNumParamsTooHigh = WEBRTC_CNG_MAX_LPC_ORDER + 1
};

enum {
    kNoSid,
    kForceSid
};

int main(int argc, char *argv[]) {
    printf("WebRtc Comfort Noise Generator\n");
    printf("舒适噪音生成器\n");
    int sample_rate_hz = 8000;//  8000, 16000,  32000,  48000, 64000
    int16_t speech_data_[640];  // Max size of CNG internal buffers.
    const size_t num_samples_10ms = (sample_rate_hz / 100);
    Buffer sid_data;
    int quality = kCNGNumParamsNormal;
    ComfortNoiseEncoder cng_encoder(sample_rate_hz, kSidNormalIntervalUpdate,
                                    quality);
    size_t ret = cng_encoder.Encode(ArrayView<const int16_t>(
            speech_data_, num_samples_10ms),
                                    kNoSid, &sid_data);
    if (ret == 0) {
        size_t size = cng_encoder.Encode(ArrayView<const int16_t>(speech_data_, num_samples_10ms),
                                         kForceSid, &sid_data);
        if (size == (quality + 1))
            printf("done \n");
        wavWrite_int16("cng.wav", speech_data_, sample_rate_hz, num_samples_10ms);
    }
    printf("按任意键退出程序 \n");
    getchar();
    return 0;
}
