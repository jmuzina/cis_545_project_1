default: all

all: atomic reduce

atomic: thr_atomic.c
	gcc thr_atomic.c -pthread -lm -o thr_atomic

reduce: thr_reduce.c
	gcc thr_reduce.c -pthread -lm -o thr_reduce

clean:
	rm -f thr_atomic
	rm -f thr_reduce