# You Spin Me Round Robin

This program simulates Round Robin scheduling, a technique that operating systems use to schedule running processes. We assume that there is a context switch of 1 when switching between 2 different processes, and that each process can either run for a fixed quantum unit or "median" quantum (the median of CPU times consumed by each process, dynamically determined as the processes are being run).

## Building

```shell
make
```

## Running
Run the command ```./rr file_name quantum_length```, where quantum_length is restricted to an integer or the string "median."

```shell
./rr processes.txt 30
./rr processes.txt median
```

## Results
Given "test.txt" below,
```shell
4
1, 10, 70
2, 20, 40
3, 40, 10
4, 50, 40
```
running ```./rr test.txt 30``` would give:
```shell
Average wait time: 82.75
Average response time: 37.00
```

running ```./rr test.txt median``` would give:
```shell
Average wait time: 81.75
Average response time: 13.75
```

## Cleaning up

```shell
make clean
```
