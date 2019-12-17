BPM = 110
SAMPLERATE = 44100
FPB = $(shell echo $(SAMPLERATE) \* 60 / $(BPM) | bc)

build/beat.flac: Makefile beat src/bumm.flac src/tack.flac
	./beat $@ $(SAMPLERATE) $$(($(FPB) / 2)) 8 2 0 src/bumm.flac 2 src/tack.flac 3 src/bumm.flac 4 src/bumm.flac 6 src/tack.flac

beat: beat.c Makefile
	gcc -lm -lsndfile $< -o $@

%_mono.flac: %.flac
	ffmpeg -i "$<" -ac 1 "$@"
