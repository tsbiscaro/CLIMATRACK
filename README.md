# CLIMATRACK
Tracking code for satellite and radar data


Data must be in short integer format (2 bytes) scaled by 100 (e.g.: 32.5 dBZ would be 3250)

Compile using the compile.sh script. Definition go into the param.txt file

To run: compile, then run "ulimit -s unlimited". Then, ./climatrack param.txt

Data order: first pixel must be lower left corner. 

Tracking method follows the definitions in "Vila, D. A., Machado, L. A. T., Laurent, H., & Velasco, I. (2008). Forecast and Tracking the Evolution of Cloud Clusters (ForTraCC) Using Satellite Infrared Imagery: Methodology and Validation, Weather and Forecasting, 23(2), 233-245" - https://doi.org/10.1175/2007WAF2006121.1

For a more complete, updated, and user friendly version we suggest the TATHU package (https://github.com/uba/tathu)
