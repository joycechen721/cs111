# You Spin Me Round Robin

This program simulates Round Robin scheduling, a technique that operating systems use to schedule running processes. We assume that there is a context switch of 1 when switching between 2 different processes, and that each process can either run for a fixed quantum unit or "median" quantum (dynamically determined as the processes are being run).

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
3, 40, 10
4, 50, 40
1, 10, 70
2, 20, 40
```
```./rr test.txt 30``` would give:
```shell
Average wait time: 82.75
Average response time: 37.00
```

## Cleaning up

```shell
make clean
```
