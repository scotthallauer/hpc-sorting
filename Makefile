# HPC program makefile
# Scott Hallauer
# 31/05/2019

JAVAC = /usr/bin/javac
OMPC = gcc
MPIC = mpicc
JFLAGS = -d bin
CFLAGS = -std=c99

all:
	$(JAVAC) $(JFLAGS) src/Generator.java
	$(OMPC) $(CFLAGS) -fopenmp src/serial_quicksort.c -o bin/serial_quicksort
	$(OMPC) $(CFLAGS) -fopenmp src/parallel_omp_quicksort.c -o bin/parallel_omp_quicksort
	$(OMPC) $(CFLAGS) -fopenmp src/parallel_omp_regsampling.c -o bin/parallel_omp_regsampling
	module load mpi/openmpi-4.0.1
	$(MPIC) $(CFLAGS) src/parallel_mpi_quicksort.c -o bin/parallel_mpi_quicksort
	$(MPIC) $(CFLAGS) src/parallel_mpi_regsampling.c -o bin/parallel_mpi_regsampling
	

serial:
	all
	sbatch jobs/job_serial.sh

omp:
	all
	sbatch jobs/job_omp_quicksort_t2.sh
	sbatch jobs/job_omp_quicksort_t4.sh
	sbatch jobs/job_omp_quicksort_t8.sh
	sbatch jobs/job_omp_regsampling_t2.sh
	sbatch jobs/job_omp_regsampling_t4.sh
	sbatch jobs/job_omp_regsampling_t8.sh

mpi:
	all
	sbatch jobs/job_mpi_quicksort_n1t2.sh
	sbatch jobs/job_mpi_quicksort_n2t4.sh
	sbatch jobs/job_mpi_quicksort_n4t8.sh
	sbatch jobs/job_mpi_regsampling_n1t2.sh
	sbatch jobs/job_mpi_regsampling_n2t4.sh
	sbatch jobs/job_mpi_regsampling_n4t8.sh

clean:
	rm bin/*
