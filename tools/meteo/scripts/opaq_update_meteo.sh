#!/bin/bash
# Wrapper script to update the OVL meteo forecast buffers
#
# The script will automatically determine the most recent GFS basetime
# to retrieve based upon the current time and a given latency. 
# Author: Bino Maiheu, (c) VITO 2013 - 2014

#PBS -V
#PBS -N OPAQ_METEO
#PBS -m e 

function print_usage {
    echo "Usage:"
    echo "  opaq_update_meteo.sh [options]"
    echo "Available options:"
    echo "  -h, --help .............. : this message"
    echo "  -l, --latency <value> ... : specify GFS latency in (whole) hours (default: 6)"
}

# =============================================================================
# Set path to scripts
# =============================================================================
MCR=/tools/matlab/2013b/x86_64
opaq_retrieve=/projects/N78H9/OPAQ/meteo/scripts/opaq_retrieve_gfs.sh
opaq_update=/projects/N78H9/OPAQ/meteo/run_opaq_import_gfs.sh

# =============================================================================
# Default values & configuration
# =============================================================================
latency=6 # can be altered via command line
step=6
fc_hor=6
gfs_repo=/projects/N78H9/OPAQ/data/gfs
db_repo=/projects/N78H9/OPAQ/data/ovldb
utc_offset=8 # 8 for CST

# configuration for netcdf conversion : table file and wgrib2
nc_table=/projects/N78H9/OPAQ/meteo/scripts/nc_vardefs.txt
nc_wgrib2=wgrib2

# specify retrieval bounding box and domain name
lonW=115
lonE=123
latS=30
latN=36
domain=yangzhou

# selection of gridcells to retrieve for ovldb
ovl_bbox="119,32;119,33"

# =============================================================================
# Checking command line
# =============================================================================
if ! options=$(getopt -o hl: -l help,latency: -- "$@")
then 
    exit 1
fi
eval set -- $options;
while [ $# -gt 0 ]; do
    case $1 in 
	-h|--help) print_usage; exit 0;;
	-l|--latency) latency=$2; shift;;
	(--) shift; break;;
	(-*) echo "$°: Error - unrecognized option $1" 1>&2; exit 1;;
	(*) break;;
    esac
    shift
done 

# =============================================================================
# Here we go
# =============================================================================
date_minus_6h=`date -d "$latency hours ago" --utc +'%Y%m%d %H'`;
fc_base=${date_minus_6h:0:8}
fc_time=${date_minus_6h:9:2}
fc_basetime="00"
if [ $fc_time -ge 6 ]; then
    fc_basetime="06"
fi
if [ $fc_time -ge 12 ]; then
    fc_basetime="12"
fi
if [ $fc_time -ge 18 ]; then
    fc_basetime="18"
fi
echo "Assuming most recent GFS run is : $fc_base - $fc_basetime"
echo "********************************************************"
echo "* Retrieving..."
echo "********************************************************"
 ${opaq_retrieve} -v -c -s $step -f $fc_hor -r $gfs_repo \
                  --domain $domain \
                  --llon $lonW --rlon $lonE --blat $latS --tlat $latN \
				  --nc --nc_table ${nc_table} --nc_wgrib2 ${nc_wgrib2} \
				  $fc_base $fc_basetime

if [ $? -ne 0 ]; then
    echo "********************************************************"
    echo "* Failed."
    echo "********************************************************"
    exit 1;
else

	if [ ! -d $db_repo/$domain ]; then
		mkdir -p $db_repo/$domain
	fi

    echo "********************************************************"
    echo "* Updating OVL databases..."
    echo "********************************************************"    
	${opaq_update} ${MCR} --domain $domain \
	                      --repo $gfs_repo/$domain \
						  --output $db_repo/$domain \
						  --tzone $utc_offset \
						  --mode ovldb --plot \
						  "$ovl_bbox" $fc_base $fc_basetime
	
	
    echo "********************************************************"
    echo "* Done."
    echo "********************************************************"
fi