This is a simple tool that does one thing: Arrange audio samples into something
bigger:

    Usage: beat OUTFILE SAMPLERATE BPM BEATS TRACKS BEAT INFILE [BEAT INFILE…]
    Example: beat beat.flac 44100 240 8 2 0 kick.flac 2 snare.flac 3 kick.flac 4 kick.flac 6 snare.flac

## Parameters

-   samplerate: all input files must have the same sample rate
-   BPM: beats per minute
-   beats: total number of beats
-   tracks: to avoid clipping, the samples are multiplied by `1/sqrt(tracks)`

The rest of the parameters are pairs of beats and file paths.

## Typical usage

I use this in combination with some other tools:

-   record samples with audacity
-   add effects to samples using `sox` (e.g. reverb or distortion)
-   arrange samples using `beat`
-   keep the parameters for `beat` in a text file
-   orchestrate everything using a Makefile

For bigger projects I may have multiple levels, where samples are first
combined into bigger samples, then have effects applied to them, and then get
combined into the final song.

This allows me to do audio production in the same way I create software: By
manipulating text files. `beat` acts as something like a linker from that
perspective.
