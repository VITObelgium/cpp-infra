#!/usr/bin/perl
#
# doxygen preprocessing filter
#
# Bino Maiheu, (c) VITO 2014
use File::Spec;
$fname = File::Spec->abs2rel( shift @ARGV );

print "/**\n \\file ";

print "$fname \n";
print $pwd;

$fname =~ /(.*?)\/.*/;
$m = $1;
print " \\ingroup $1 \n";
open F,"<",$fname || die "Error while filtering: $!";
print "*/\n";
while (<F>) { print "$_"; }

