#!/usr/bin/perl
# =================================================================
#  OPAQ toolkit for air quality prediction
#  Perl script to retrieve NCEP FNL analysis files
#
# Author: Bino Maiheu, (c) VITO 2012-2013
# =================================================================
use Getopt::Long;
use File::Copy;
use DateTime;

# =================================================================
# Default configuration
# =================================================================
$ucar_url      = "https://rda.ucar.edu/cgi-bin/login";
$ucar_login    = "bino.maiheu\@vito.be";
$ucar_passwd   = "ymTKIpFJ";
$ucar_cookie   = "auth.rda_ucar_edu";
$data_url_root = "http://rda.ucar.edu/data/ds083.2/grib2";
@hours         = ( "00", "06", "12", "18" );
undef $help;
undef $start_date;
undef $end_date;
undef $out_path;
# =================================================================
# Parse command line arguments
# =================================================================
if ( !GetOptions( "help!"     => \$help,
		  "user=s"    => \$ucar_login,
                  "pass=s"    => \$ucar_passwd,
		  "from=s"    => \$start_date,
		  "to=s"      => \$end_date,
                  "outpath=s" => \$out_path ) ) {
    die "opaq_retrieve_fnl.pl: error parsing command line options, try --help\n";
}	
printUsage() if ( $help );

				  
# =================================================================
#  Build the filelist
# =================================================================
# parse left over dates from command line
@years  = ();
@months = ();
@days   = ();
foreach( @ARGV ) {
    ( $y,$m,$d ) = ( $_ =~ /(\d\d\d\d)(\d\d)(\d\d)/ );
    push @years, $y;
    push @months, $m;
    push @days, $d;
}

# now parse the start and end date options
if ( $start_date && $end_date ) {
    ( $y1,$m1,$d1 ) = ( $start_date =~ /(\d\d\d\d)(\d\d)(\d\d)/ );
    ( $y2,$m2,$d2 ) = ( $end_date   =~ /(\d\d\d\d)(\d\d)(\d\d)/ );
    
    my $start_dt = DateTime->new( year => $y1, month => $m1, day => $d1 );
    my $end_dt   = DateTime->new( year => $y2, month => $m2, day => $d2 );		
    for ( my $dt = $start_dt->clone(); $dt <= $end_dt; $dt->add( days => 1 ) ) {
	push @years, $dt->year();
	push @months, $dt->month();
	push @days, $dt->day();
    }
} 
die "opaq_retrieve_fnl.pl: error, please specify a date, see --help\n" if ( ! scalar( @years ) );


# =================================================================
# Get cookie with login credentials
# =================================================================
print "****************************\n";
print "* Logging into UCAR system *\n";
print "****************************\n";
if ( -f $ucar_cookie ) { unlink $ucar_cookie; }
open VN, "wget -V |" or die 'opaq_retrieve_fnl.pl: cannot find wget';
$vn = (<VN> =~ /^GNU Wget (\d+)\.(\d+)/) ? (100 * $1 + $2) : 109;
close(VN);
$syscmd  = ($vn > 109 ? 'wget --no-check-certificate' : 'wget');
$syscmd .= " -nv -O /dev/null --save-cookies $ucar_cookie --post-data=\"email=$ucar_login&password=$ucar_passwd&action=login\" $ucar_url";
system( $syscmd );		  
if ( ! -f $ucar_cookie ) {
    die "opaq_retrieve_fnl.pl: unable to set cookie...";
} else {
    print "---> cookie retrieved, login ok...\n";
}

# =================================================================
# Retrieving the data
# =================================================================
print "****************************\n";
print "* Retrieving FNL data ...  *\n";
print "****************************\n";
for ( my $i=0; $i<=$#years; $i++ ) {
	printf "Retrieving %04d-%02d-%02d...\n", $years[$i], $months[$i], $days[$i];
	foreach $hr ( @hours ) {
		$fname   = sprintf "fnl_%04d%02d%02d_%02d_00", $years[$i],$months[$i],$days[$i],$hr;
		$syscmd  = "wget -nv -N --load-cookies $ucar_cookie";
		$syscmd .= sprintf " $data_url_root/%04d/%04d.%02d/$fname", $years[$i], $years[$i], $months[$i];
		system( $syscmd );
		
		if ( -f $fname ) {			
			move( $fname, $out_path."/".$fname ) if ( $out_path );		
		} else {
			print "opaq_retrieve_fnl.pl: warning, unable to retrieve $fname, skipping\n";
		}
		
	}
}
if ( -f $ucar_cookie ) { unlink $ucar_cookie; }

sub printHeader() {
	print "OPAQ Operational Prediction of Air Quality Toolkit\n";
	print "Bino Maiheu, (c) VITO 2013\n\n";
	print "This script retrieves the NCEP final analysis data from the servers for a given period.\n";
	print "We make use of the grib2 data, see http://rda.ucar.edu/datasets/ds083.2/index.html for\n";	
	print "details. Note we use ds083.2 \n\n";
}

sub printUsage() {
	printHeader();
	print "Usage:\n";
	print "  opaq_retrieve_fnl.pl [options] [YYYYMMDD]\n";
	print "Retrieval options\n";
	print "  --help                       : this message\n";
	print "  --user <name>                : provide user email address (register at http://rda.ucar.edu/)\n";
	print "  --pass <password>            : provide password\n";
	print "  --outpath <folder>           : store to this directory\n";
	print "  --from <YYYYMMDD>            : retrieve fnl data from this date\n";
	print "  --to   <YYYYMMDD>            : .. to this date\n";
	exit 0;	
}
