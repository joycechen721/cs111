# Hash Hash Hash
In this lab, we are adding mutex locks to implement concurrent hash tables. v1 of the hash table is concerned with accuracy, whereas v2 is concerned with both performance and accuracy.

## Building
To create the executable, run:
```shell
make
```

## Running
The executable takes two command link arguments: -t changes the number of threads to use (default 4), and -s changes the number of hash table entries to add per thread (default 25,000). 
```shell
./hash-table-tester -t 8 -s 50000
```

## First Implementation
In the `hash_table_v1_add_entry` function, I added a mutex around the entire function, such that all threads except the caller will sleep until the item compfinishes getting added.

### Performance
```shell
#to run
./hash-table-tester -t 8 -s 50000

#results
Hash table v1: 1,440,436 usec
  - 0 missing
```
Version 1 is a little slower/faster than the base version. As it locks the entire function region, each add operation by a thread will finish in its entirety before another add operation starts.

## Second Implementation
In the `hash_table_v2_add_entry` function, I added a mutex around the line `SLIST_INSERT_HEAD(list_head, list_entry, pointers);`. This is because the insertion of a hash table entry can cause a race condition. If a context switch occurs during insertion, and another thread inserts a new entry, the original thread will not see the new entry and will overwrite it with its entry.

### Performance
```shell
#to run
./hash-table-tester -t 8 -s 50000

#results
Hash table v2: 487,385 usec
  - 0 missing
```
For this version, I added lock in the function around a specific critical section for optimal performance.


## Cleaning up
```shell
make clean
```
Also, destroy the lock using ``pthread_mutex_destroy(&(hash_table->lock));`` in the cleanup function.