/* This is a sample execution of the program */

wolverine@ubuntu:~/C/GPS$ gcc -o gps gps.c 
wolverine@ubuntu:~/C/GPS$ ./gps
enter the number of flows: 5
enter the total bandwidth: 100
enter number of time intervals: 3
enter the bandwidth per flow: 
flow 0	20
flow 1	12
flow 2	12
flow 3	20
flow 4	16
enter the amount of data per flow for interval 0: 
flow 0	20
flow 1	12
flow 2	20
flow 3	20
flow 4	30
total bandwidth is: 100000000 
sum is: 80 
allocated bandwidth: 
flow 0: 25000000
flow 1: 15000000
flow 2: 15000000
flow 3: 25000000
flow 4: 20000000
data pending flow 0: 20000
data pending flow 1: 12000
data pending flow 2: 20000
data pending flow 3: 20000
data pending flow 4: 30000
temp: 20000.000000 temp2: 20000
flow 0 ends at time 0.000800
temp: 12000.000000 temp2: 12000
flow 1 ends at time 0.000800
temp: 12000.000000 temp2: 12000
temp: 20000.000000 temp2: 20000
flow 3 ends at time 0.000800
temp: 16000.000000 temp2: 16000
total bandwidth is: 100000000 
sum is: 28 
allocated bandwidth: 
flow 2: 42857142
flow 4: 57142857
data pending flow 2: 8000
data pending flow 4: 14000
temp: 8000.000000 temp2: 8000
flow 2 ends at time 0.000187
temp: 10666.666853 temp2: 10666
total bandwidth is: 100000000 
sum is: 16 
allocated bandwidth: 
flow 4: 100000000
data pending flow 4: 3333
temp: 1333.332960 temp2: 1333
after interval 1, flow status:	 1	1	1	1	0	enter the amount of data per flow for interval 5: 
flow 5	20
flow 6	0
flow 7	0
flow 8	0
flow 9	0
total bandwidth is: 100000000 
sum is: 36 
allocated bandwidth: 
flow 5: 55555555
flow 9: 44444444
data pending flow 5: 20000
data pending flow 9: 1999
temp: 2498.750000 temp2: 2498
temp: 1999.000000 temp2: 1999
flow 9 ends at time 0.000045
total bandwidth is: 100000000 
sum is: 20 
allocated bandwidth: 
flow 5: 100000000
data pending flow 5: 17501
temp: 17501.000000 temp2: 17501
flow 5 ends at time 0.000175
after interval 1, flow status:	 1	1	1	1	1	enter the amount of data per flow for interval 10: 
flow 10	20
flow 11	0
flow 12	0
flow 13	0
flow 14	0
total bandwidth is: 100000000 
sum is: 0 
allocated bandwidth: 
Floating point exception (core dumped)
wolverine@ubuntu:~/C/GPS$ D
