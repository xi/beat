example.flac: example.beat $(shell grep -o '[^ ]*\.flac' example.beat)
	beat $@ $(shell cat $< | grep -v '^#')

%_fx.flac: %.flac
	sox $< $@ reverb

.PHONY: clean
clean:
	rm -f example.flac *_fx.flac
