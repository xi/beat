#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

#define MIN(A, B) ((A) < (B) ? (A) : (B))

struct context {
    int samplerate;
    int frames_per_beat;  // samplerate * 60 / bpm
    int frames;
    int buf_cur;
    size_t buf_len;
    float *buf;
    float *buf2;
};

void add_file_at_beat(const char *path, int beat, struct context ctx) {
    int ibs = 1024;
    int pos = beat * ctx.frames_per_beat;
    float fbuf[ibs];
    SF_INFO sfinfo;
    SNDFILE *sndfile = sf_open(path, SFM_READ, &sfinfo);

    // assert sfinfo.samplerate == ctx.samplerate
    // assert sfinfo.channels == 1

    while (1) {
        int count = sf_readf_float(sndfile, fbuf, ibs);
        count = MIN(count, ctx.frames - pos - 1);
        int rel_pos = pos - ctx.buf_cur * ctx.buf_len;

        if (count <= 0) break;

        for (int i = 0; i < count; ++i) {
            pos += 1;
            rel_pos += 1;
            if (rel_pos >= 2 * ctx.buf_len) {
                printf("dropping %s at %i\n", path, pos);
                sf_close(sndfile);
                return;
            } else if (rel_pos >= ctx.buf_len) {
                ctx.buf2[rel_pos - ctx.buf_len] += fbuf[i];
            } else {
                ctx.buf[rel_pos] += fbuf[i];
            }
        }
    }

    sf_close(sndfile);
}

int main(int argc, char **argv) {
    if (argc < 6 || argc % 2 != 1) {
        printf("Usage: beat OUTFILE SAMPLERATE FRAMES_PER_BEAT BEATS BEAT INFILE [BEAT INFILE…]\n");
        exit(1);
    }

    struct context ctx;

    ctx.samplerate = atoi(argv[2]);
    ctx.frames_per_beat = atoi(argv[3]);
    ctx.frames = atoi(argv[4]) * ctx.frames_per_beat;

    ctx.buf_len = MIN(ctx.frames / 4, 1 << 18);
    ctx.buf_cur = 0;

    float buf[ctx.buf_len];
    memset(buf, 0, ctx.buf_len * sizeof(float));
    ctx.buf = buf;

    float buf2[ctx.buf_len];
    memset(buf2, 0, ctx.buf_len * sizeof(float));
    ctx.buf2 = buf2;

    SF_INFO sfinfo;
    sfinfo.channels = 1;
    sfinfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
    sfinfo.frames = ctx.frames;
    sfinfo.samplerate = ctx.samplerate;
    sfinfo.sections = 1;
    sfinfo.seekable = 1;

    SNDFILE *sndfile = sf_open(argv[1], SFM_WRITE, &sfinfo);

    for (int i = 5; i + 1 < argc; i += 2) {
        int beat = atoi(argv[i]);
        char *path = argv[i + 1];

        if (beat * ctx.frames_per_beat >= (ctx.buf_cur + 1) * ctx.buf_len) {
            sf_writef_float(sndfile, ctx.buf, ctx.buf_len);
            memset(ctx.buf, 0, ctx.buf_len * sizeof(float));

            float *tmp = ctx.buf;
            ctx.buf = ctx.buf2;
            ctx.buf2 = tmp;
            ctx.buf_cur += 1;
        }

        add_file_at_beat(path, beat, ctx);
    }

    int rest = ctx.frames - ctx.buf_cur * ctx.buf_len;
    if (rest > ctx.buf_len) {
        sf_writef_float(sndfile, ctx.buf, ctx.buf_len);
        sf_writef_float(sndfile, ctx.buf2, rest - ctx.buf_len);
    } else {
        sf_writef_float(sndfile, ctx.buf, rest);
    }

    sf_close(sndfile);
}
