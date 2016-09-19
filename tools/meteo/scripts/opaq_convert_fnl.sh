#!/bin/bash

# Script to convert FNL grib2 files to netcdf via some table
# Bino Maiheu, (c) bino.maiheu@vito.be


function print_usage {
    echo "Usage:"
    echo " opaq_convert_fnl.sh [options] start_date end_date"
    echo "Where:"
    echo "  start_date <YYYYMMDD> ....... : start date for conversion"
    echo "  end_date <YYYYMMDD> ......... : end date for conversion"
    echo "Available options:"
    echo "  -h, --help .................. : this message"
    echo "  -v, --verbose ............... : verbode output"
    echo "  -r, --repo <path> ........... : set FNL repository path"
    echo "  -o, --output <path> ......... : set output path (default .)"
    echo "Bounding box options:"
    echo "  --lonW <value> .............. : set west longitude (default -180)"
    echo "  --lonE <value> .............. : set east longitude (default 180)"
    echo "  --latS <value> .............. : set south latitude (default -90)"
    echo "  --latN <value> .............. : set north latitude (default 90)"
    echo "  --domain <name> ............. : set name for domain (def. world)"
    echo "NetCDF Conversion options:"
    echo "  --nc_table <fname> .......... : provide nc_table file (def. nc_vardef.txt)"
    echo "  --nc_wgrib2 <loc> ........... : location of wgrib2 binary (def. wgrib2)"
    echo ""
    echo "OPAQ was created by Bino Maiheu, (c) VITO 2013-2014"
    echo "Contact: bino.maiheu@vito.be"
}

# ==========================================================================
# Default configuration
# ==========================================================================
# -- Specify bounding box defaults
domain=world
LonW=-180
LonE=180
LatS=-90
LatN=90
use_bbox=0

# -- Default FNL repository, the year will be added automatically
repo_dir=/projects/N78H9/OPAQ/data/fnl
output_dir=.

# -- Verbose output
verbose=0

# -- Conversion options
nc_table="nc_vardef.txt"
nc_wgrib2="wgrib2"

# -- Internal date format
fmt="%Y%m%d"

# ==========================================================================
# Checking command line
# ==========================================================================
if ! options=$(getopt -o r:o:hv -l help,verbose,repo:,output:,lonW:,lonE:,latS:,latN:,domain:,nc_table:,nc_wgrib2: -- "$@")
then
    exit 1
fi
eval set -- $options
while [ $# -gt 0 ]; do
    case $1 in
        --lonW) LonW=$2; use_bbox=1; shift;;
        --lonE) LonE=$2; use_bbox=1; shift;;
        --latS) LatS=$2; use_bbox=1; shift;;
        --latN) LatN=$2; use_bbox=1; shift;;
        --domain) domain=$2; use_bbox=1; shift;;
        --nc_table) nc_table=$2; shift;;
        --nc_wgrib2) nc_wgrib2=$2; shift;;
	-o|--output) output_dir=$2; shift;;
	-r|--repo) repo_dir=$2; shift;;
        -v|--verbose) verbose=1;;
        -h|--help) print_usage; exit -1 ;;
        (--) shift; break;;
        (-*) echo "$0: Error - unrecognized option $1" 1>&2; exit 1;;
        (*) break;;
    esac
    shift
done

# filling other arguments
if [ $# -ne 2 ]; then
    echo "Error: please specify start and end date (see --help)..."
    exit 1
fi
start_date=`date +$fmt -d "$1"`
end_date=`date +$fmt -d "$2"`


# ==========================================================================
# Known domains
# ==========================================================================
case $domain in
    china)
	echo "Valid domain name : $domain, setting bounding box"
	LonW=71
	LonE=137
	LatS=17
	LatN=54
	;;
esac

if [ $verbose -eq 1 ]; then
    echo "Bounding box : "
    echo " Longitude : $LonW - $LonE"
    echo " Latitude  : $LatS - $LatN"
fi

# ==========================================================================
# Some checks
# ==========================================================================
if [ ! -d $repo_dir ]; then
    echo "***error: repository dir does not exist"
    exit 1
fi

v_match=`cat ${nc_table} | grep "^#\!" | sed s/^#\!// | sed 's/^[ \t]*//;s/[ \t]*$//'`


# if we have a v_match, use it otherwise dont use the -match option...
curr_date=$start_date
while [[ "$curr_date" -le "$end_date" ]]; do
    echo "Handling $curr_date"
    year=`echo $curr_date | cut -c1-4`
    
    if [ ! -d ${output_dir}/${year} ]; then
	echo "Making output folder ${output_dir}/${year}"
	mkdir -p ${output_dir}/${year}
    fi

    nc_file=${output_dir}/${year}/fnl.${domain}.${curr_date}.nc
    
    if [ -f ${nc_file} ]; then
	echo "+++ warning: nc file exists (${nc_file}), cleaning..."
	rm -f ${nc_file}
    fi

    # looping over the hours in the day...
    for hr in 00 06 12 18; do 

	grbname=${repo_dir}/${year}/fnl_${curr_date}_${hr}_00
	echo "Processing ${grbname} ..."

	if [ ${use_bbox} -eq 1 ]; then
            # use unnamed pipes for transferring the subsetted grib file to the netcdf conversion
	    if [ -z "$v_match" ]; then
		${nc_wgrib2} ${grbname} \
		    -small_grib ${LonW}:${LonE} ${LatS}:${LatN} - | \
		    ${nc_wgrib2} - -nc_table ${nc_table} -append -netcdf ${nc_file}  > /dev/null 2<&1
	    else
		${nc_wgrib2} ${grbname} \
		    -small_grib ${LonW}:${LonE} ${LatS}:${LatN} - | \
		    ${nc_wgrib2} - -match "${v_match}" -nc_table ${nc_table} -append -netcdf ${nc_file}  > /dev/null 2<&1
	    fi
	else
	    if [ -z "$v_match" ]; then
		${nc_wgrib2} $grbname -nc_table ${nc_table} -append -netcdf ${nc_file}  > /dev/null 2<&1
	    else 
		${nc_wgrib2} $grbname -match "${v_match}" \
		    -nc_table ${nc_table} -append -netcdf ${nc_file} > /dev/null 2<&1	    
	    fi
	fi

    done

    curr_date=`date +$fmt -d "$curr_date + 1 day"`
done


echo "All done."
