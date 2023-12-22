beat: beat.c
	gcc -lm -lsndfile $< -o $@
