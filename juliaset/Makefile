JULIA_LIB = julia.c

sequential:
	gcc -o sequential_julia sequential_julia.c $(JULIA_LIB)
clean-sequential:
	rm sequential_julia julia.bmp
1d:
	smpicc -O3 1D_parallel_julia.c -o 1D_parallel_julia $(JULIA_LIB)
clean1d:
	rm 1D_parallel_julia julia1d.bmp
2d:
	smpicc -O3 2D_parallel_julia.c -o 2D_parallel_julia $(JULIA_LIB)
clean2d:
	rm 2D_parallel_julia julia2d.bmp
smpi:
	rm smpitmp-*
clean: smpi clean-sequential clean1d clean2d