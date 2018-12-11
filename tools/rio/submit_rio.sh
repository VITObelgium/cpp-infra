#!/bin/bash
# 
# RIO submission script for running on the cluster
# Bino Maiheu, bino.maiheu@vito.be
# (c) VITO 2010-2011


# ========================================================================
# RIO config : select your settings here
# ========================================================================
pol='no2'
agg='1h'
gis='CorineID'
grid='4x4'
year=2007  # adjust days in februari below here for leap years !!
base='/home/maiheub/devel/rio/matlab'

# ========================================================================
# Some paths & other
# ========================================================================
month=(01 02 03 04 05 06 07 08 09 10 11 12)
mname=(jan feb mar apr may jun jul aug sep oct nov dec)
days_in_mon=(31 28 31 30 31 30 31 31 30 31 30 31)
matlabroot=/pf16/uhi/depend/matlabR2008b

# ========================================================================
# go !!
# ========================================================================
cd $base

# checking presence of output folder, will not overwrite !
if [ -d "./results/${pol}_val" ]; then	
	while true; do
    read -p "Output folder exists already, clean now [yes/no] ?" yn
    case $yn in
        [Yy]* )
          echo "Cleaning up...";
          rm -rf ./results/${pol}_val;
          mkdir -p ./results/${pol}_val;
          break;;
        [Nn]* ) 
          echo "Quitting, please clean manually..."; 
          exit;;
        * ) echo "Please answer yes or no.";;
    esac
	done
else
	mkdir -p ./results/${pol}_val;
fi

# running over the months
for i in ${!month[*]}; do
	
	echo "Handling RIO run for ${mname[$i]}"	
	script=rio_${pol}_${agg}_${year}-${mname[$i]}.pbs
	if [ $agg == "1h" ]; then
		from_date=01.${month[$i]}.${year}-00;
		to_date=${days_in_mon[$i]}.${month[$i]}.${year}-23;
  else
		from_date=01.${month[$i]}.${year};
		to_date=${days_in_mon[$i]}.${month[$i]}.${year};
  fi
	
	echo " - Building pbs script : $script";		
	cat > $script  <<EOF
#!/bin/bash
#PBS -V
#PBS -N rio_${pol}-${mname[$i]}
#PBS -o rio-${mname[$i]}.out
#PBS -e rio-${mname[$i]}.err
#PBS -m e

echo "Running on \`hostname\`"
umask 002
cd $base
./run_rio.sh $matlabroot $pol $agg $gis $grid $from_date $to_date

EOF

	echo " - Submitting RIO run ${mname[$i]} : $from_date $to_date";	
	qsub $script
done

# do a final qstat to see everything on the cluster
qstat -1n
