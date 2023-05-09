# Round Robin Scheduler

This basic C program simulates round robin scheduling by calculating the corresponding waiting_time and response_time for each process. It takes a text file and quantum length as arguments.

## Usage
```bash
# 1st arg must be text file with process info
# 2nd arg must be desired quantum length
./rr processes.txt 3
```
## Processes File Format
The first line must be a single integer specifying the number of processes in the file. 
The following lines specify each processes metadata in the following format: PID, Arrival_Time, Burst_Time
Example:
```
4
1, 0, 7
2, 2, 4
3, 4, 1
4, 5, 4
```

## Build
```bash
make
```
## Clean
```bash
make clean
```
## Test
```bash
python -m unittest
```
