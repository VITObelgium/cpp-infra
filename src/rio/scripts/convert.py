#!/usr/bin/python

import sys, getopt, datetime, re
import numpy as np


def main(argv):
    inputfile = ''
    outputfile = ''
    stationsfile = None
    count = None
    try:
        opts, args = getopt.getopt(argv, "hi:o:s:c:", ["ifile=", "ofile=", "stations", "count"])
    except getopt.GetoptError:
        print('convert.py -i <inputfile> -o <outputfile> -s <stationsfile> -c <count>')
        sys.exit(2)
    if not opts:
        print('convert.py -i <inputfile> -o <outputfile> -s <stationsfile>  -c <count>')
        sys.exit()
    for opt, arg in opts:
        if not opt or opt == '-h':
            print('convert.py -i <inputfile> -o <outputfile> -s <stationsfile>  -c <count>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
        elif opt in ("-s", "--stations"):
            stationsfile = arg
        elif opt in ("-c", "--count"):
            count = int(arg)

    convert(inputfile, outputfile, stationsfile, count)


def convert(input_file, output_file, stationsfile, count):
    print(" Loading File:"),
    lines = open_input_file_to_list(input_file)

    if stationsfile is not None:
        print(" Load stations file")
        with open(stationsfile) as fp:
            unique_stations = fp.read().split(",")
        fp.close
        # keep only lines with given stations
        lines = [item for item in lines if item[0] in unique_stations]

    print(" Parse dates:"),
    lowest_date, highest_date, unique_stations = get_info(lines)
    base_line = [-9999] * 29

    result_list = list()
    print("Start processing: "),
    for station in unique_stations:
        station_sublist = [item for item in lines if item[0] == station]
        if count is None or count < station_sublist.__len__():
            result_dict = dict()
            for line in station_sublist:
                update_or_insert_result_line(base_line, line, result_dict, station)
            insert_nodata_dates(base_line, highest_date, lowest_date, result_dict, station)
            write_dict_to_result(result_dict, result_list)
        print("."),
    print("")
    print("writing output ")
    output = open(output_file, 'w')
    for line in result_list:
        output.write("%s\n" % ' '.join(map(str, line)))
    print(" ")
    print("Done!")


def write_dict_to_result(result_dict, result_list):
    for key in sorted(result_dict.keys()):
        val = result_dict[key]
        hour_values = [x for x in val[5:29] if x > -9999]
        if hour_values:
            hour_values = [float(i) for i in hour_values]
            avg = format(sum(hour_values) / len(hour_values), '.2f')
            max_val = max(hour_values)
            m8_val = calculate_m8_val(hour_values)

            val[2] = -9999 if max_val == 0 else max_val  # M1
            val[3] = -9999 if m8_val == 0 else m8_val  # M8
            val[4] = -9999 if float(avg) < 0.001 else avg  # DA
        else:
            val[2] = -9999
            val[3] = -9999
            val[4] = -9999
        result_list.append(val)


def calculate_m8_val(hour_values):
    values = sorted(hour_values)
    if len(values) >= 8:
        return format(sum(values[-8:]) / 8, '.2f')
    return -9999


def open_input_file_to_list(input_file):
    lines = []
    i = 0
    with open(input_file, 'r') as infile:
        # parse file line by line to prevent out of memory on large files. Keep only values that are used
        for line in infile:
            line = [x for x in line.split('\t')]
            # stationCode, Concentration, Date
            keepers = [4, 11, 13]
            line = [line[index] for index in keepers]
            lines.append(line)
            i += 1
            if i > 100000:
                i = 0
                print('.'),
    infile.close
    print("")
    # remove header and blank line at end
    lines.pop(0)
    lines.pop(-1)
    return lines


def update_or_insert_result_line(base_line, line, result_dict, station):
    date = get_parsed_date(line[2])
    check_date = '{}{:02d}{:02d}'.format(date.year, date.month, date.day)
    if check_date not in result_dict:
        insert_nodata_line(base_line, check_date, result_dict, station)
    hour = date.hour
    result_dict[check_date][hour + 5] = line[1]


def insert_nodata_dates(base_line, highest_date, lowest_date, result_dict, station):
    low_date = lowest_date
    while low_date < highest_date:
        check_date = '{}{:02d}{:02d}'.format(low_date.year, low_date.month, low_date.day)
        if check_date not in result_dict:
            insert_nodata_line(base_line, check_date, result_dict, station)
        low_date = low_date + datetime.timedelta(days=1)


def insert_nodata_line(base_line, check_date, result_dict, station):
    new_line = list(base_line)
    new_line[0] = station
    new_line[1] = check_date
    result_dict[check_date] = new_line


def get_info(lines):
    lowest_date = None
    highest_date = None
    unique_stations = []
    i = 0
    for line in lines:
        date_field = line[2]
        date = get_parsed_date(date_field)
        if lowest_date is None:
            lowest_date = date
            highest_date = date
        if date < lowest_date:
            lowest_date = date
        elif date > highest_date:
            highest_date = date

        unique_stations.append(line[0])
        i += 1
        if i > 100000:
            i = 0
            print('.'),
    print("")
    return lowest_date, highest_date, sorted(set(unique_stations))


def get_parsed_date(date_field):
    p = re.compile('[- :]')
    date = datetime.datetime(*map(int, p.split(date_field)))
    return date


if __name__ == "__main__":
    main(sys.argv[1:])
