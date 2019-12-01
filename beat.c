#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

struct context {
    int samplerate;
    int frames_per_beat;  // samplerate * 60 / bpm
    int frames;
    float *buf;
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

        if (pos + count >= ctx.frames) {
            count = ctx.frames - pos - 1;
        }

        if (count <= 0) break;

        for (int i = 0; i < count; ++i) {
            pos += 1;
            ctx.buf[pos] += fbuf[i];
        }
    }

    sf_close(sndfile);
}

void write_file(const char *path, struct context ctx) {
    SF_INFO sfinfo;

    sfinfo.channels = 1;
    sfinfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
    sfinfo.frames = ctx.frames;
    sfinfo.samplerate = ctx.samplerate;
    sfinfo.sections = 1;
    sfinfo.seekable = 1;

    SNDFILE *sndfile = sf_open(path, SFM_WRITE, &sfinfo);
    sf_writef_float(sndfile, ctx.buf, ctx.frames);
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

    float buf[ctx.frames];
    memset(buf, 0, ctx.frames * sizeof(float));
    ctx.buf = buf;

    for (int i = 5; i + 1 < argc; i += 2) {
        int beat = atoi(argv[i]);
        char *path = argv[i + 1];
        add_file_at_beat(path, beat, ctx);
    }

    write_file(argv[1], ctx);
}
