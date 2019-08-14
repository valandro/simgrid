JULIA_LIB = julia.c

sequential:
	gcc -o sequential_julia sequential_julia.c $(JULIA_LIB)
1-D:
	smpicc -O3 1D_parallel_julia.c -o 1D_parallel_julia $(JULIA_LIB)
clean:
	rm sequential_julia julia.bpm 1D_parallel_julia