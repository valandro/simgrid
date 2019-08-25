JULIA_LIB = julia.c

sequential:
	gcc -o sequential_julia sequential_julia.c $(JULIA_LIB)
clean-sequential:
	rm sequential_julia julia.bpm
smpi:
	rm smpitmp-*
1d:
	smpicc -O3 1D_parallel_julia.c -o 1D_parallel_julia $(JULIA_LIB)
clean-1d:
	rm 1D_parallel_julia julia1d.bpm
clean: smpi clean-sequential clean-1d