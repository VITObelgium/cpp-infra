#!/bin/bash

# ==========================================================================
# Script to retrieve GFS forcasts through the NOMADS server and convert to 
# netcdf
# 
#   opaq_retrieve_gfs.sh YYYYMMDD [00,06,12,18]
#
# Where the first argument gives the base date for the forecast, i.e. the
# date the forecast was started. The second argument gives the UTC forecast
# time (4 forecasts are run every day)...
#  
# Construct the url via :
#   http://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_hd.pl
#   You can select the variables and the levels as well as a bounding box
#
# Author: Bino Maiheu, (c) 2013-2014 VITO
#
# Documentation on GFS & wgrib2
#  - http://www.emc.ncep.noaa.gov/?doc=doc
#  - http://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/
#
# Changelog
#  - 2013.09.05 : - created
#  - 2014.03.20 : - added the export functionality to netcdf via wgrib2
#                 - added domain name functionality, note that the domain name
#                   will now appear in the output path as well !!
#  - 2014.04.01 : - leaving out the basedate tag (${fc_base}) out of the output 
#                   path for the NetCDF files
#  - 2014.04.03 : - added the --clean switch to clean up the grib2 files
# ==========================================================================

function print_usage {
    echo "Usage:"
    echo " opaq_retrieve_gfs.sh [options] YYYYMMDD XX"
    echo "Where:"
    echo "  YYYYMMDD   : forecast basedate, use \"now\" for today (UTC)"
    echo "  XX         : forecast basetime (00,06,12 or 18) in UTC"
    echo "Available options: "
    echo "  -h, --help .............. : this message"
    echo "  -v, --verbose ........... : show config after startup"
	echo "  -c, --clean ............. : clean up grib2 files after finish"
    echo "  -s, --step <3,6,...>..... : set the forcast step in hours (default 6)"
    echo "  -f, --fc_hor <days> ..... : set forecast horizon in days (default 3)"
    echo "  -r, --repo <path> ....... : set repository location (without domain name)"
    echo "Bounding box options: "
    echo "  --llon <value> .......... : set left longitude value (def -180)"
    echo "  --rlon <value> .......... : set left longitude value (def. 180)"
    echo "  --blat <value> .......... : set bottom lat (def. -90)"
    echo "  --tlat <value> .......... : set top latitude (def. 90)"
    echo "  --domain <name> ......... : set name for domain (def. world)"
    echo "NetCDF Conversion options:"
    echo "  --nc .................... : convert to netcdf (def. do not)"
    echo "  --nc_table <fname> ...... : provide nc_table file (def. nc_vardef.txt)"
    echo "  --nc_wgrib2 <loc> ....... : location of wgrib2 binary (def. wgrib2)"
    echo ""
    echo "OPAQ was created by Bino Maiheu, (c) VITO 2013-2014"
    echo "Contact: bino.maiheu@vito.be"
}

# ==========================================================================
# Default configuration
# ==========================================================================

# -- Forecast horizon
# define the timesteps to take (minimum resolution is 3 hours, normally 6 hours)
step=6

# define the forecast horizon in days (6 means we go untill day+6)
fc_hor=3

# -- Specify bounding box. Note that we make the left hand start at -180, 
#    this means however that in the output, the longitude will be from 
#    180 -> 540, so you will need to subtract 360 from this !! Check this!
left_lon=-180
right_lon=180
bottom_lat=-90
top_lat=90
domain=world

# -- Repository for GFS retrievals, note that the year will be added as a 
#    subfolder in this base folder
repo_dir=/projects/N78H9/OPAQ/data/gfs

# -- Email warning, when send_email is 1, an email will be sent, otherwise
#    retrieval error messages will be displayed on stdout
send_email=1
email_addr="bino.maiheu@vito.be"
email_from="OPAQ SYSTEM"

# -- Verbose output by default ?
verbose=0

# -- Cleanup after finish converting to netcdf
cleanup=0

# -- Conversion options
nc=0
nc_table="nc_vardef.txt"
nc_wgrib2="wgrib2"

# ==========================================================================
# Checking command line
# ==========================================================================
if ! options=$(getopt -o s:r:f:hvc -l help,clean,verbose,step:,repo:,fc_hor,llon:,rlon:,blat:,tlat:,domain:,nc,nc_table:,nc_wgrib2: -- "$@")
then
    exit 1
fi
eval set -- $options
while [ $# -gt 0 ]; do
    case $1 in
	-s|--step)  step=$2; shift;;
	-r|--repo)  repo_dir=$2; shift;;
	-f|--fc_hor) fc_hor=$2; shift;;
	--llon) left_lon=$2; shift;;
	--rlon) right_lon=$2; shift;;
	--blat) bottom_lat=$2; shift;;
	--tlat) top_lat=$2; shift;;
	--domain) domain=$2; shift;;
	--nc) nc=1;;
	--nc_table) nc_table=$2; shift;;
	--nc_wgrib2) nc_wgrib2=$2; shift;;
	-v|--verbose) verbose=1;;
	-c|--clean) cleanup=1;;
	-h|--help) print_usage; exit -1 ;;
	(--) shift; break;;
	(-*) echo "$0: Error - unrecognized option $1" 1>&2; exit 1;;
	(*) break;;
    esac
    shift
done

# filling other arguments
if [ $# -ne 2 ]; then
    echo "Error: please specify forecast basetime (see --help)..."
    exit 1
fi
fc_basedate=$1
fc_basetime=$2

# ==========================================================================
# Compute the retrieval hours and setup the time
# ==========================================================================
hours=`seq -f %02g 0 $step $(expr $(expr 24 \* $(expr $fc_hor + 1) ) - $step)`
if [[ $fc_basedate == "now" || $fc_basedate == "today" ]]; then
    fc_basedate=`date +%Y%m%d`
fi
year=`echo $fc_basedate | cut -c1-4`

if [ ! -d $repo_dir ]; then
    echo "***error: repository dir does not exist"
    exit 1
fi

run_start=`date +%Y.%m.%d-%H:%M:%S`
fc_base="${fc_basedate}.${fc_basetime}"
mkdir -p ${repo_dir}/${domain}/${year}/${fc_base}
logfile=${repo_dir}/${domain}/${year}/${fc_base}/gfs_retrieval.${run_start}.log

cat > $logfile <<EOF
OPAQ GFS RETRIEVAL:
 Started           : ${run_start}
 Contact           : ${email_addr}
 Repository        : ${repo_dir}
 Timestep          : ${step}
 Horizon           : day+${fc_hor}
 Forecast base     : ${fc_base}
 Hours to retrieve : ${hours} 
 Bounding box
 - left longitude  : ${left_lon}
 - right longitude : ${right_lon}
 - bottom latitude : ${bottom_lat}
 - top latitude    : ${top_lat} 
 - domain name     : ${domain}
WGET LOG
EOF

# show configuration
if [ $verbose -eq 1 ]; then 
    cat $logfile
fi

# ==========================================================================
#  Define variable list for the -match command in wgrib2
#  Make use of the matching functionality of wgrib2, note that this list
#  has to be compatible with the variable list in the NOMADS retrieval
#  as well as the nc_vardef_gfs.txt table provided
#
# --> update: replaced this by an entry in the header of the nc_table file
#             such that we keep the FNL and GFS netcdf structure information
#             in one and the same place...
# ==========================================================================
#v_match=":(HPBL|TCDC|TMP|RH|UGRD|VGRD|HGT):((1000|975|950|925|900|850|800|750|700) mb|2 m above ground|10 m above ground|surface|low cloud layer|middle cloud layer|high cloud layer|convective cloud layer|entire atmosphere \(considered as a single layer\))"
v_match=`cat ${nc_table} | grep "^#\!" | sed s/^#\!// | sed 's/^[ \t]*//;s/[ \t]*$//'`

# ==========================================================================
# Looping over the retrieval hours...
# ==========================================================================
nc_name="${repo_dir}/${domain}/${year}/gfs.${domain}.${fc_base}.nc" 
if [ -f $nc_name ]; then
    echo "+++warning: deleting existing file: $nc_name"
    rm -f $nc_name
fi

for hr in $hours; do
    echo "Retrieving GFS ${fc_basedate}, run ${fc_basetime}h UT, horizon +${hr}h..."
    read -d '' url <<EOF
http://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_hd.pl?file=\
gfs.t${fc_basetime}z.mastergrb2f${hr}&\
lev_surface=on&\
lev_2_m_above_ground=on&\
lev_10_m_above_ground=on&\
lev_1000_mb=on&\
lev_975_mb=on&\
lev_950_mb=on&\
lev_925_mb=on&\
lev_900_mb=on&\
lev_850_mb=on&\
lev_800_mb=on&\
lev_750_mb=on&\
lev_700_mb=on&\
lev_entire_atmosphere_%5C%28considered_as_a_single_layer%5C%29=on&\
lev_low_cloud_layer=on&\
lev_middle_cloud_layer=on&\
lev_high_cloud_layer=on&\
lev_convective_cloud_layer=on&\
var_HPBL=on&\
var_TCDC=on&\
var_TMP=on&\
var_RH=on&\
var_UGRD=on&\
var_VGRD=on&\
var_HGT=on&\
subregion=&\
leftlon=${left_lon}&\
rightlon=${right_lon}&\
toplat=${top_lat}&\
bottomlat=${bottom_lat}&\
dir=%2Fgfs.${fc_basedate}${fc_basetime}%2Fmaster
EOF
    grbname=${repo_dir}/${domain}/${year}/${fc_base}/gfs.${fc_base}.${hr}.grb2 

    # upon request of the nomads.ncep.noaa.gov site, pause a little before retrieving
    sleep 2
    
    # go !
    wget -nv -a $logfile -O $grbname $url
    if [ $? -ne 0 ]; then
	if [ $send_email -eq 1 ]; then
	    mail -a "From:${email_from}" -s "[opaq_retrieve_gfs] unable to retrieve ${fc_base}" $email_addr <<EOF
An error occurred retrieving GFS forecast
fc_base: $fc_base
hr : $hr
year : $year
url: $url
EOF
	else
	    echo "*** An error occurred retrieving GFS forecast..."
	fi

	rm -Rf ${repo_dir}/${domain}/${year}/${fc_base}
	exit 1;
    else
        # succesful retrieval, append to nc file if needed
	if [ $nc -eq 1 ]; then 
	    ${nc_wgrib2} ${grbname} \
                         -match "${v_match}" \
                         -nc_table ${nc_table} \
                         -append \
                         -netcdf ${nc_name} > /dev/null 2>&1

	    if [ $? -eq 0 ]; then
		echo "Succesfully appended to `basename ${nc_name}`"
	    else
		# oops
		if [ $send_email -eq 1 ]; then
		    mail -a "From:${email_from}" -s "[opaq_retrieve_gfs] unable to add to netcdf" $email_addr <<EOF
An error occurred adding GFS forecast to netcdf
fc_base: $fc_base
hr : $hr
year : $year
url: $url
nc_name: ${nc_name}
nc_table: ${nc_table}
nc_wgrib2: ${nc_wgrib2}
hostname: $HOSTNAME
EOF
		else
		    echo "*** An error occurred appending GFS forecast to $nc_name...";
		fi
	    fi
	fi
    fi
done

# ==========================================================================
# Cleanup grib2 retrievals ?
# ==========================================================================
if [ $cleanup -eq 1 ]; then 
	echo "Cleaning up the grib2 files..."
	rm -Rf ${repo_dir}/${domain}/${year}/${fc_base}
fi

# ==========================================================================
# Closing time...
# ==========================================================================
stop_date=`date +%Y%m%d-%H:%M:%S`
cat >> $logfile <<EOF
OPAQ GFS RETRIEVAL:
 Finished at : ${stop_date}
DONE
EOF
echo "All done, have a nice day :-)"
