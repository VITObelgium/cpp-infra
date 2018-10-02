#!/usr/bin/env python

# -*- coding: utf-8 -*-
"""
convert_param_wk.py

Script to convert the RIO parameters files to the new format (XML), 
this one is a special edition for the "wk" aggregation

Created on Thu Apr 12 15:46:13 2018

@author: MAIHEUB
"""
import sys
import numpy as np
import pandas as pd
import argparse
import xml.etree.ElementTree

from xml.etree.ElementTree import Element, SubElement, tostring
from xml.dom import minidom

def prettify(elem):
    """Return a pretty-printed XML string for the Element.
    """
    rough_string = tostring(elem, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="  ")


def make_str( nums ):
    return '{:s}'.format(' '.join(['{:.6f}'.format(x) for x in nums]))
    

# Command line arguments
p = argparse.ArgumentParser(description='Convert RIO parameters to new format')
p.add_argument( "basedir" )

p.add_argument( "-c", "--cnf", default="v3.6", help="specify configuration, default=v3.6")
p.add_argument( "-p", "--pol", default="pm10",    help="specify pollutant, default pm10")
p.add_argument( "-g", "--gis", default="clc06d", help="specify gis proxy, default clc06d")
p.add_argument( "-n", "--name", default="STATION", help="specify column name in station file for stations" )
p.add_argument( "-s", "--s_out", default="sp_corr.xml", help="specify output for spatial correlation")
p.add_argument( "-t", "--t_out", default="trendmodel.xml", help="specify output for ipol model (trend)")

args = p.parse_args()

print('Welcome to convert_param.py')
print('Processing : {}'.format( args.basedir ) )


# import xml setup file in FORTRAN deployment
setup = xml.etree.ElementTree.parse(args.basedir + '/param/rio_setup.xml')
avg_trend_cnf = setup.getroot().findall("./Configuration[@name='" + args.cnf + 
                                    "']/Pollutant[@name='"+ args.pol + 
                                    "']/Driver[@name='"+ args.gis +
                                    "']/Aggregation[@name='wk"+
                                    "']/avgtrend" )[0].attrib
    
std_trend_cnf = setup.getroot().findall("./Configuration[@name='" + args.cnf + 
                                    "']/Pollutant[@name='"+ args.pol + 
                                    "']/Driver[@name='"+ args.gis +
                                    "']/Aggregation[@name='wk"+
                                    "']/stdtrend" )[0].attrib

if not avg_trend_cnf or not std_trend_cnf:
    sys.exit( "unable to find avg_trend or std_trend in xml config..." )    

print( 'avg_trend config: ' )   
print( avg_trend_cnf )
print( 'std_trend config: ' )   
print( std_trend_cnf )


# importing station_file
st_file = args.basedir + '/stations/' + args.cnf + '/' + args.pol + '/' + args.pol + '_stations_info_GIS_' +  args.gis + '.txt'
print( 'Importing stations from ' + st_file )
st_info = pd.read_csv( st_file, delim_whitespace=True )


paramDir = args.basedir + '/param/' + args.cnf + '/' + args.pol

# importing stat_param
# always using the "all aggregation", no week/weekend difference
file =  paramDir + '/stat_param/' + 'avg_' + args.pol + '_all_agg_time-wk.txt'
print( 'Importing stat_param from ' + file )
avg  = pd.read_csv( file, delim_whitespace=True, header=None )
file = paramDir + '/stat_param/' + 'std_' + args.pol + '_all_agg_time-wk.txt'
print( 'Importing stat_param from ' + file )
std  = pd.read_csv( file, delim_whitespace=True, header=None )

# importing trend information - e.g. avg_err_trend_pm10_clc06d_all_agg_time-1h
file = paramDir + '/trend/' + 'avg_trend_' + args.pol + '_' + args.gis + '_all_agg_time-wk.txt'
print( 'Importing trend from ' + file )
avg_trend = pd.read_csv( file, delim_whitespace=True, header=None )
file = paramDir + '/trend/' + 'std_trend_' + args.pol + '_' + args.gis + '_all_agg_time-wk.txt'
print( 'Importing trend from ' + file )
std_trend = pd.read_csv( file, delim_whitespace=True, header=None )
file = paramDir + '/trend/' + 'avg_err_trend_' + args.pol + '_' + args.gis + '_all_agg_time-wk.txt'
print( 'Importing trend from ' + file )
avg_err_trend = pd.read_csv( file, delim_whitespace=True, header=None )
file = paramDir + '/trend/' + 'std_err_trend_' + args.pol + '_' + args.gis + '_all_agg_time-wk.txt'
print( 'Importing trend from ' + file )
std_err_trend = pd.read_csv( file, delim_whitespace=True, header=None )

# importing spatial correlation - e.g. p_long_pm10_clc06d_agg_time-1h
file = paramDir + '/spatial_corr/' + 'p_long_' + args.pol + '_' + args.gis + '_agg_time-wk.txt'
print( 'Importing p_long from ' + file )
p_long = pd.read_csv( file, delim_whitespace=True, header=None )
file = paramDir + '/spatial_corr/' + 'p_short_' + args.pol + '_' + args.gis + '_agg_time-wk.txt'
print( 'Importing p_short from ' + file )
p_short = pd.read_csv( file, delim_whitespace=True, header=None )

# now write xml for trendmodel
xml = Element( 'trendmodel' )
classEl = SubElement( xml, 'class' ) 
classEl.text = 'polytrend'
confEl = SubElement( xml, 'config')
avgEl = SubElement(confEl, 'avg_trend')
el = SubElement( avgEl, 'ref_level')
el.text = avg_trend_cnf['ref_level']
el = SubElement( avgEl, 'x_lo')
el.text = avg_trend_cnf['indic_lo']
el = SubElement( avgEl, 'x_hi')
el.text = avg_trend_cnf['indic_hi']

stdEl = SubElement(confEl, 'std_trend')
el = SubElement( stdEl, 'ref_level')
el.text = std_trend_cnf['ref_level']
el = SubElement( stdEl, 'x_lo')
el.text = std_trend_cnf['indic_lo']
el = SubElement( stdEl, 'x_hi')
el.text = std_trend_cnf['indic_hi']

el = SubElement( xml, 'start_date')
el.text = 'XXXXX'
el = SubElement( xml, 'end_date')
el.text = 'XXXXX'

print( 'Wriring trend xml...' )

# write xml for trend
trEl = SubElement( xml, 'trend' )
el  = SubElement( trEl, 'avg' )
el.text = make_str( avg_trend.iloc[0,:].values.tolist() )
el  = SubElement( trEl, 'std' )
el.text = make_str( std_trend.iloc[0,:].values.tolist() )
el  = SubElement( trEl, 'avg_err' )
el.text = make_str( avg_err_trend.iloc[0,:].values.tolist() )
el  = SubElement( trEl, 'std_err' )
el.text = make_str( std_err_trend.iloc[0,:].values.tolist() )   
        
stpEl = SubElement( xml, 'stat_param')
for index, row in st_info.iterrows():    
    stEl = SubElement( stpEl, 'station', { 'name': row[args.name] })    

    avgEl = SubElement( stEl, 'avg' )
    avgEl.text = '{:.6f}'.format(avg.iloc[index,1])
    stdEl = SubElement( stEl, 'std' )
    stdEl.text = '{:.6f}'.format(std.iloc[index,1])
        

# writing trend output file
filename = args.t_out 
with open( filename, 'w' ) as f:
    f.write( prettify(xml) )    

print( 'Wrote ' + filename )


# write xml for sp corr
# now write xml for trendmodel
xml = Element( 'correlationmodel' )
classEl = SubElement( xml, 'class' ) 
classEl.text = 'exp_spcorr'
rangeEl = SubElement( xml, 'short_range', { 'active': 'yes'} )
rangeEl.text = '20.'

spEl = SubElement( xml, 'param' )

el_l = SubElement( spEl, 'long' )
el = SubElement( el_l, 'a' )
el.text = '{:.6f}'.format(np.exp(p_long.iloc[0,1]))
el = SubElement( el_l, 'tau' )
el.text = '{:.6f}'.format( -1./p_long.iloc[0,0] )
        
el_s = SubElement( spEl, 'short' )
el = SubElement( el_s, 'a' )
el.text = '{:.6f}'.format(p_short.iloc[0,0])
el = SubElement( el_s, 'b' )
el.text = '{:.6f}'.format(p_short.iloc[0,1])
    

# writing outputfile
filename = args.s_out
with open( filename, 'w' ) as f:
    f.write( prettify(xml) )
    
print( 'Wrote ' + filename )
