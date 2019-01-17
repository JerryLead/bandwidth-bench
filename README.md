# Bandwidth Bench
You divide a dataset into b-sized blocks. You access the b-sized blocks randomly.
How does your memory/disk bandwidth change depending on b? This tiny project answers that question.

## Usage
- $ make

For measuring memory bandwidth. One cacheline is 64B:
- $ test mem &lt;blockSizeInCacheLines&gt; &lt;totalSizeInMB&gt;

For measuring disk bandwidth:
1. Write a temporary file:
- $ test write 0 &lt;totalSizeInMB&gt;
2. Drop cached pages in linux:
- $ echo 3 | sudo tee /proc/sys/vm/drop_caches
3. Read the temporary file in blocks:
- $ test disk &lt;blockSizeInCacheLines&gt; &lt;totalSizeInMB&gt;