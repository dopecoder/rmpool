#!/bin/bash

# create a output directory
mkdir -p data
mkdir -p data/sorting-benchmarks
mkdir -p data/ram-bench
mkdir -p data/sysbench


# SORTING-BENCHMARK

printf "Started SORTING-BENCHMARK"

cd sorting-benchmarks

cmake CMakeLists.txt
make

./runtime_local.sh

cd ..

printf "Finished SORTING-BENCHMARK\n"


# RAM-BENCH

printf "Running RAM-BENCH"

cd ram-bench

./build_and_run_local.sh

cd ..

printf "Finished RAM-BENCH\n"

# SYSBENCH

printf "Started SYSBENCH"

sysbench --test=memory --memory-block-size=64K --memory-total-size=100G --num-threads=1 run > ./data/sysbench/sysbench-64K-local

sysbench --test=memory --memory-block-size=1M --memory-total-size=100G --num-threads=1 run > ./data/sysbench/sysbench-1M-local

sysbench --test=memory --memory-block-size=4M --memory-total-size=100G --num-threads=1 run > ./data/sysbench/sysbench-4M-local

printf "Finished SYSBENCH\n"

## Use perf to display the number of pagefaults for an example program such as malloc

# MEMORY TEST

# memory_test 0 21 > mem-test-local
# LD_PRELOAD=../preloadlib/bin/preloadlib.so memory_test 0 21 > mem-test-remote

# MAGICK CONVERT

# LD_PRELOAD=../preloadlib/bin/preloadlib.so time convert img.jpg img.png > convert-1-remote 2>&1
# time convert img.jpg img.png > convert-1-local 2>&1

# LD_PRELOAD=../preloadlib/bin/preloadlib.so time convert input.png -colorspace RGB +sigmoidal-contrast 11.6933 \
#   -define filter:filter=Sinc -define filter:window=Jinc -define filter:lobes=3 \
#   -resize 400% -sigmoidal-contrast 11.6933 -colorspace sRGB output.png > convert-2-remote 2>&1

# time convert input.png -colorspace RGB +sigmoidal-contrast 11.6933 \
#   -define filter:filter=Sinc -define filter:window=Jinc -define filter:lobes=3 \
#   -resize 400% -sigmoidal-contrast 11.6933 -colorspace sRGB output.png > convert-2-local  2>&1
