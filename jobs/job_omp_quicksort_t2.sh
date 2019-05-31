#!/bin/sh

# The line below indicates which accounting group to log your job against - must be its
#SBATCH --account=icts

# The line below selects the group of nodes you require - you will all run on curie
#SBATCH --partition=curie

# The line below means you need 1 worker node and a total of 2 cores
#SBATCH --nodes=1 --ntasks=1
# To evenly split over two nodes use: --ntasks-per-node=2

# The line below indicates the wall time your job will need, 10 hours for example. NB, this is a mandatory directive!
# Note that 10:00 is 10 minutes
#SBATCH --time=1:00:00

# The line below means your job will be canceled if it consumes more than 4GB of RAM per requested core. Max 9000.
#SBATCH --mem-per-cpu=2000

# A sensible name for your job, try to keep it short
#SBATCH --job-name="OpenMP Quicksort Job (2 Threads)"

# Modify the lines below for email alerts. Valid type values are NONE, BEGIN, END, FAIL, REQUEUE, ALL
#SBATCH --mail-user=HLLSCO001@myuct.ac.za
#SBATCH --mail-type=BEGIN,END,FAIL

#SBATCH --output=../tests/out_omp_quicksort_t2.txt
#SBATCH --error=../tests/err_omp_quicksort_t2.txt

# NB, for more information read https://computing.llnl.gov/linux/slurm/sbatch.html

# Use module to gain easy access to software, typing module avail lists all packages.
# Example:
# module load python/anaconda-python-2.7

export OMP_NUM_THREADS=2

for j in 10 100 500 1000 5000 10000 50000 100000 500000 1000000 5000000 10000000 25000000 50000000 75000000 100000000
do
    java -cp ../bin Generator ../bin/numbers_omp_quicksort_t2.txt $j
    for i in {1..3}
    do
        echo "$j [$i]"
        ../bin/parallel_omp_quicksort ../bin/numbers_omp_quicksort_t2.txt
    done
done

rm ../bin/numbers_omp_quicksort_t2.txt
