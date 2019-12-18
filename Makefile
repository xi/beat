BPM = 110
SAMPLERATE = 44100

build/beat.flac: Makefile beat src/bumm.flac src/tack.flac
	./beat $@ $(SAMPLERATE) $$(($(BPM) * 2)) 8 2 0 src/bumm.flac 2 src/tack.flac 3 src/bumm.flac 4 src/bumm.flac 6 src/tack.flac

beat: beat.c Makefile
	gcc -lm -lsndfile $< -o $@
