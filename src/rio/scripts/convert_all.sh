#!/bin/bash

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

# path to the RIO fortran installation
#baseDir=/home/maiheub/Development/rio-v4.0
baseDir=/home/maiheub/Development/rio-4.0-rivm

# conversion function
function conv_param {
    cnf=$1
    pol=$2
    agg=$3
    gis=$4

    ${SCRIPTPATH}/convert_param.py -c $cnf -p $pol -a $agg -g $gis \
                       -s ./${cnf}/spcorr_${pol}_${agg}_${gis}.xml \
                       -t ./${cnf}/trend_${pol}_${agg}_${gis}.xml \
                       $baseDir
}

# running the conversion
#for pol in pm10 no2 o3; do 
#    conv_param v2010 ${pol} 1h CorineID
#    conv_param v2010 ${pol} da CorineID
#done

for pol in pm10 no2 o3; do
    for cnf in v2016 v2016t; do 
        for agg in 1h da; do
            conv_param ${cnf} ${pol} ${agg} clc12
            conv_param ${cnf} ${pol} ${agg} clc12a
        done
    done
done


# v2016
#conv_param v2016 pm10 1h clc06d
#conv_param v2016 pm10 da clc06d

#conv_param v2016 pm25 1h clc06d
#conv_param v2016 pm25 da clc06d

# skipping O3....

# v3.6
#conv_param v3.6 so2 1h clc06d 
#conv_param v3.6 so2 da clc06d 

#conv_param v3.6 pm10 1h clc06d
#conv_param v3.6 pm10 da clc06d

#conv_param v3.6 pm25 1h clc06d
#conv_param v3.6 pm25 da clc06d

#conv_param v3.6 no2 1h clc06d
#conv_param v3.6 no2 da clc06d

#conv_param v3.6 o3 1h clc06d
#conv_param v3.6 o3 da clc06d

#conv_param v3.6 bc 1h clc06d
#conv_param v3.6 bc da clc06d

