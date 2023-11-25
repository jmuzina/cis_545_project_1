# Compiling:
Make is used for compilation.

- Atomic: `make atomic`
- Reduce: `make reduce`
- All: `make all`
- Clean: `make clean`

Defaults to `make all` if no args proivded.

# Running:
`./thr_(atomic|reduce) <number of threads> <upper bound>` where `<number of threads>` and `<upper bound>` are integer powers of 2, and `<number of threads> <= <upper bound>`.