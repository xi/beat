CFLAGS = -std=c99 -Wall -O2 `pkg-config --cflags sndfile`
LDFLAGS = -s -lm `pkg-config --libs sndfile`

beat: beat.c
	gcc $< -o $@ ${CFLAGS} ${LDFLAGS}
