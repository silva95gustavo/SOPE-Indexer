FLAGS = -Wall
MKDIR_P = mkdir -p

make: out_dir bin/index bin/sw bin/csc
out_dir:
	${MKDIR_P} bin
bin/index: index.c index.h bin/sw bin/csc
	gcc $(FLAGS) index.c -o bin/index
bin/sw: sw.c sw.h
	gcc $(FLAGS) sw.c -o bin/sw
bin/csc: csc.c csc.h
	gcc $(FLAGS) csc.c -o bin/csc