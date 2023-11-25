# Compiling:
- Atomic: `gcc thr_atomic.c -lm -o thr_atomic`
- Reduce: `gcc thr_reduce.c -lm -o thr_reduce`

Important: must include -lm flag to link Math library to the code, for using the `pow()` function.

# Running:
- Atomic: `./thr_atomic <number of threads> <upper bound>` where `<number of threads>` and `<upper bound>` are integer powers of 2, and `<number of threads> <= <upper bound>`.
- Reduce: