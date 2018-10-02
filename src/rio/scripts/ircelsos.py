#!/usr/bin/env python
"""
Usage: 
 ircelsos.py --start 2018-06-06-01 --hours 24 --shape ./shapes/belgium.shp --outdir .

The script assumes the timestamps in the start argument to be before the hour in UTC
so retreiving 2018-06-01-23 --hours 24 will retrieve the full day 2018-06-01, doing
2018-06-01-00 --hours 24 will retrieve the first hour of 2018-05-31 to and including 
the first hour of 2018-06-01 (00:00 - 01:00).

Note that hours goes back in time !

Authors: Guy Driesen, Bino Maiheu, (c) VITO 2018
"""
import datetime
import fiona
import math
import os
import requests
import shapely.geometry
import sys
from argparse import ArgumentParser


# Configuration
stationNameLuchtbal = '42M802'
sosPollutantNames = [
    #'Benzene',
    'Black Carbon',
    #'Carbon Monoxide',
    'Nitrogen dioxide',
    'Nitrogen monoxide',
    'Ozone',
    'Particulate Matter < 2.5 µm',
    'Particulate Matter < 10 µm',
    #'Sulphur dioxide',
]

sosMeteoWindDirection = 'wind direction'
sosMeteoWindSpeed = 'wind speed (scalar)'
sosMeteoTemperature = 'temperature'
sosMeteoNames = [
    sosMeteoWindDirection,
    sosMeteoWindSpeed,
    sosMeteoTemperature,
]

pollutantFileNames = {
    #'Benzene' : '',
    'Black Carbon' : 'bc.txt',
    #'Carbon Monoxide' : '',
    'Nitrogen dioxide' : 'no2.txt',
    'Nitrogen monoxide' : 'no.txt',
    'Ozone' : 'o3.txt',
    'Particulate Matter < 2.5 µm' : 'pm25.txt',
    'Particulate Matter < 10 µm' : 'pm10.txt',
    #'Sulphur dioxide' : '',
}
meteoFileName = 'Meteotype1invoer.txt'

# DateTime formatter for argument parser
def mkdatetime(datetimestr):
    return datetime.datetime.strptime(datetimestr, "%Y-%m-%d-%H")


# Parse arguments
arg_parser = ArgumentParser(description='query data from Ircel sos for RIO & IFDM')

arg_parser.add_argument('--start', type=mkdatetime, default=datetime.datetime.utcnow(), help='First hour to query from the sos in UTC')
arg_parser.add_argument('--hours', type=int, default=1, help='number of hours to query')
arg_parser.add_argument('--shape', type=str, default='belgium.shp', help='the shapefile to use to limit the number of stations')
arg_parser.add_argument('--outdir', type=str, default='.', help='the directory to place the output files')

args = arg_parser.parse_args()

# Empty all files
for fileName in list(pollutantFileNames.values()) + [meteoFileName, ]:
    open(os.path.join(args.outdir, fileName), 'w').close()

# Determine time range
timeTo = args.start
timeFrom = timeTo - datetime.timedelta(hours=args.hours-1)

# Timestamps in the SOS are after the hour
# the script assumes input of timestamps before the hour, i.e. 2018-05-01-23 --hours 24 will retreive the full day 2018-05-01
timeFromSos = timeFrom + datetime.timedelta(hours=1)
timeToSos = timeTo + datetime.timedelta(hours=1)

# Open shapefile
shape = fiona.open(args.shape)
feature = next(iter(shape)) # only one feature in shapefile
poly = shapely.geometry.shape(feature['geometry'])


# Get station list from sos
stations = requests.get('http://geo.irceline.be/sos/api/v1/stations')


# Select the stations in shape
# https://gis.stackexchange.com/questions/208546/check-if-a-point-falls-within-a-multipolygon-with-python
selectedStations = []
for station in stations.json():
    station['geometry']['coordinates'] = station['geometry']['coordinates'][:-1] # 3D point, 3rd dimension NaN, Fiona shape doesn't like this
    point = shapely.geometry.shape(station['geometry'])
    if point.within(poly):
        selectedStations.append(station)

# Get and parse timeseries for te selected stations
meteoData = {}
for station in selectedStations:
    stationid = station['properties']['id']
    stationName = station['properties']['label'][0:6]

    stationurl = 'http://geo.irceline.be/sos/api/v1/stations/' + str(stationid)
    stationinfo = requests.get(stationurl)

    for timeserieId in stationinfo.json()['properties']['timeseries']:
        sosPhenomenonName = stationinfo.json()['properties']['timeseries'][timeserieId]['phenomenon']['label']
        if (sosPhenomenonName not in (sosPollutantNames + sosMeteoNames)):
            continue

        timeserieurl = 'http://geo.irceline.be/sos/api/v1/timeseries/' + timeserieId + '/getData'
        timeserieparams = {
            'timespan' : timeFromSos.strftime("%Y-%m-%dT%H:00:00+00:00") + '/' + timeToSos.strftime("%Y-%m-%dT%H:00:00+00:00"),
            'generalize' : 'false',
            'expanded' : 'true',
            'format' : 'flot',
            'locale' : 'en',
        }
        timeserie = requests.get(timeserieurl, params=timeserieparams)

        if (sosPhenomenonName in sosPollutantNames):
            pollutantFile = open(os.path.join(args.outdir, pollutantFileNames[sosPhenomenonName]), 'a')
            timeserieValueId = 0
            timeserieValues = len(timeserie.json()[timeserieId]['values'])
            for day in list(range((timeTo.day - timeFrom.day) + 1)):
                date = timeFrom + datetime.timedelta(days=day)
                date = datetime.datetime(date.year, date.month, date.day) # set time to 00:00
                line = stationName + '\t' + date.strftime('%Y%m%d')
                line += '\t' + str(-9999) + '\t' + str(-9999) + '\t' + str(-9999) # M1 M8 DA
                for hour in list(range(0,24)):
                    if (timeserieValueId == timeserieValues):
                        line += '\t' + str(-9999)
                        continue
                    value = timeserie.json()[timeserieId]['values'][timeserieValueId]
                    if (timeserieValueId == 0):
                        delta = datetime.datetime.utcfromtimestamp(value[0] // 1000) - date - datetime.timedelta(hours=hour) - datetime.timedelta(hours=1)
                        if (delta.seconds != 0):
                            line += '\t' + str(-9999)
                            continue
                    line += '\t' + str(-9999 if math.isnan(float(value[1])) else value[1])
                    timeserieValueId += 1
                pollutantFile.write(line + '\n')
            pollutantFile.close()

        elif (sosPhenomenonName in sosMeteoNames):
            if (stationName == stationNameLuchtbal): # Meteo only form Luchtbal
                if (stationName not in meteoData):
                    meteoData[stationName] = {}

                if ('geometry' not in meteoData[stationName]):
                    meteoData[stationName]['geometry'] = station['geometry']

                if ('data' not in meteoData[stationName]):
                    meteoData[stationName]['data'] = {}

                for value in timeserie.json()[timeserieId]['values']:
                    if (value[0] not in meteoData[stationName]['data']):
                        meteoData[stationName]['data'][value[0]] = {}
                    meteoData[stationName]['data'][value[0]][sosPhenomenonName] = value[1]


# Write meteo file
meteoFile = open(os.path.join(args.outdir, meteoFileName), 'a')

timeZone = 0 # hour of measurement is given in UTC
measurementHeight = 30 # Luchtbal meteostation

latitude = meteoData[stationNameLuchtbal]['geometry']['coordinates'][1]
longitude = meteoData[stationNameLuchtbal]['geometry']['coordinates'][0]

measurementValues = len(meteoData[stationNameLuchtbal]['data'])
meteoLines = ''
for timestamp in meteoData[stationNameLuchtbal]['data']:
    temperature = meteoData[stationNameLuchtbal]['data'][timestamp][sosMeteoTemperature]
    windSpeed = meteoData[stationNameLuchtbal]['data'][timestamp][sosMeteoWindSpeed]
    windDirection = meteoData[stationNameLuchtbal]['data'][timestamp][sosMeteoWindDirection]
    
    # Check if the datavalues could be valid
    if (math.isnan(float(temperature)) or
        math.isnan(float(windSpeed))  or
        math.isnan(float(windDirection)) or
        windSpeed < 0):
        measurementValues -= 1
        continue

    timestamputc = datetime.datetime.utcfromtimestamp(timestamp // 1000) - datetime.timedelta(hours=1)
    temperature += 273.15 # convert to Kelvin
    windSpeedWE = - windSpeed * math.sin(math.radians(windDirection))
    windSpeedNS = - windSpeed * math.cos(math.radians(windDirection))

    meteoLines +=        str(windSpeedWE)
    meteoLines += '\t' + str(windSpeedNS)
    meteoLines += '\t' + str(measurementHeight)
    meteoLines += '\t' + str(temperature)
    meteoLines += '\t' + str(timestamputc.strftime('%Y'))
    meteoLines += '\t' + str(timestamputc.strftime('%m'))
    meteoLines += '\t' + str(timestamputc.strftime('%d'))
    meteoLines += '\t' + str(timestamputc.strftime('%H'))
    meteoLines += '\t' + str(latitude)
    meteoLines += '\t' + str(longitude)
    meteoLines += '\t' + str(timeZone)
    meteoLines += '\n'

meteoFile.write(str(measurementValues) + '\n')
meteoFile.write(meteoLines)
meteoFile.close()

