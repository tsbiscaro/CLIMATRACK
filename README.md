# CLIMATRACK
Tracking code for satellite and radar data


Data must be in short integer format (2 bytes) scaled by 100 (e.g.: 32.5 dBZ would be 3250)

Compile using the compile.sh script. Definition go into the param.txt file

To run: compile, then run "ulimit -s unlimited". Then, ./climatrack param.txt

Data order: first pixel must be lower left corner. 
