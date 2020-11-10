
# QUICKSORT

# LD_PRELOAD=../preloadlib/bin/preloadlib.so ./bin/quicksort-1-thousand > qs-1-thousand-remote
# ./bin/quicksort-1-thousand > qs-1-thousand-local

# LD_PRELOAD=../preloadlib/bin/preloadlib.so ./bin/quicksort-10-thousand > qs-10-thousand-remote
# ./bin/quicksort-10-thousand > qs-10-thousand-local

# LD_PRELOAD=../preloadlib/bin/preloadlib.so ./bin/quicksort-100-thousand > qs-100-thousand-remote
# ./bin/quicksort-100-thousand > qs-100-thousand-local

# LD_PRELOAD=../preloadlib/bin/preloadlib.so ./bin/quicksort-1-million > qs-1-million-remote
# ./bin/quicksort-1-million > qs-1-million-local

# MAGICK CONVERT

# LD_PRELOAD=../preloadlib/bin/preloadlib.so time convert img.jpg img.png > convert-1-remote 2>&1
# time convert img.jpg img.png > convert-1-local 2>&1

# LD_PRELOAD=../preloadlib/bin/preloadlib.so time convert input.png -colorspace RGB +sigmoidal-contrast 11.6933 \
#   -define filter:filter=Sinc -define filter:window=Jinc -define filter:lobes=3 \
#   -resize 400% -sigmoidal-contrast 11.6933 -colorspace sRGB output.png > convert-2-remote 2>&1

# time convert input.png -colorspace RGB +sigmoidal-contrast 11.6933 \
#   -define filter:filter=Sinc -define filter:window=Jinc -define filter:lobes=3 \
#   -resize 400% -sigmoidal-contrast 11.6933 -colorspace sRGB output.png > convert-2-local  2>&1

# SYSBENCH

# LD_PRELOAD=../preloadlib/bin/preloadlib.so sysbench --test=memory --memory-block-size=64K --memory-total-size=100G --num-threads=1 run > sysbench-64K-remote
# sysbench --test=memory --memory-block-size=64K --memory-total-size=100G --num-threads=1 run > sysbench-64K-local

# LD_PRELOAD=../preloadlib/bin/preloadlib.so sysbench --test=memory --memory-block-size=1M --memory-total-size=100G --num-threads=1 run > sysbench-1M-remote
# sysbench --test=memory --memory-block-size=1M --memory-total-size=100G --num-threads=1 run > sysbench-1M-local

# LD_PRELOAD=../preloadlib/bin/preloadlib.so sysbench --test=memory --memory-block-size=4M --memory-total-size=100G --num-threads=1 run > sysbench-4M-remote
# sysbench --test=memory --memory-block-size=4M --memory-total-size=100G --num-threads=1 run > sysbench-4M-local

# MEMORY TEST

memory_test 0 21 > mem-test-local
LD_PRELOAD=../preloadlib/bin/preloadlib.so memory_test 0 21 > mem-test-remote


