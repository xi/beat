#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include <math.h>

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BUFSIZE (1 << 18)

struct context {
    float buf[BUFSIZE];
    struct context *next;
};

int samplerate;
int frames_per_beat;  // samplerate * 60 / bpm
int frames;
int buf_cur;
float factor;
struct context *ctx;

void add_file_at_beat(const char *path, int beat) {
    int ibs = 1024;
    int pos = beat * frames_per_beat;
    int rel_pos = pos - buf_cur * BUFSIZE;
    float fbuf[ibs];
    SF_INFO sfinfo;
    SNDFILE *infile = sf_open(path, SFM_READ, &sfinfo);

    // assert sfinfo.samplerate == samplerate
    // assert sfinfo.channels == 1

    while (1) {
        int count = sf_readf_float(infile, fbuf, ibs);
        count = MIN(count, frames - pos - 1);

        if (count <= 0) break;

        for (int i = 0; i < count; ++i) {
            pos += 1;
            rel_pos += 1;
            if (rel_pos >= 2 * BUFSIZE) {
                printf("dropping %s at %i\n", path, pos);
                sf_close(infile);
                return;
            } else if (rel_pos >= BUFSIZE) {
                ctx->next->buf[rel_pos - BUFSIZE] += fbuf[i] * factor;
            } else {
                ctx->buf[rel_pos] += fbuf[i] * factor;
            }
        }
    }

    sf_close(infile);
}

int _sf_writef_float(SNDFILE *sndfile, float *buf) {
    int size = MIN(frames - buf_cur * BUFSIZE, BUFSIZE);
    if (size <= 0) {
        return 0;
    }
    return sf_writef_float(sndfile, buf, size);
}

int main(int argc, char **argv) {
    if (argc < 6 || argc % 2 != 0) {
        printf("Usage: beat OUTFILE SAMPLERATE BPM BEATS TRACKS BEAT INFILE [BEAT INFILE…]\n");
        exit(1);
    }

    samplerate = atoi(argv[2]);
    frames_per_beat = samplerate * 60 / atoi(argv[3]);
    frames = atoi(argv[4]) * frames_per_beat;
    factor = 1 / sqrt(atoi(argv[5]));

    buf_cur = 0;

    ctx = (struct context *)malloc(sizeof(struct context));
    memset(ctx->buf, 0, BUFSIZE * sizeof(float));

    ctx->next = (struct context *)malloc(sizeof(struct context));
    memset(ctx->next->buf, 0, BUFSIZE * sizeof(float));
    ctx->next->next = ctx;

    SF_INFO sfinfo;
    sfinfo.channels = 1;
    sfinfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
    sfinfo.frames = frames;
    sfinfo.samplerate = samplerate;
    sfinfo.sections = 1;
    sfinfo.seekable = 1;

    SNDFILE *outfile = sf_open(argv[1], SFM_WRITE, &sfinfo);

    for (int i = 6; i + 1 < argc; i += 2) {
        int beat = atoi(argv[i]);
        char *path = argv[i + 1];

        while (beat * frames_per_beat >= (buf_cur + 1) * BUFSIZE) {
            _sf_writef_float(outfile, ctx->buf);
            memset(ctx->buf, 0, BUFSIZE * sizeof(float));

            ctx = ctx->next;
            buf_cur += 1;
        }

        add_file_at_beat(path, beat);
    }

    _sf_writef_float(outfile, ctx->buf);
    _sf_writef_float(outfile, ctx->next->buf);

    free(ctx->next);
    free(ctx);
    sf_close(outfile);
}
