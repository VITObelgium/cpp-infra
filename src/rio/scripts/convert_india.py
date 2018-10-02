#!/usr/bin/python

import sys, getopt, datetime, re
import pandas as pd
from dateutil.parser import parse
import os


def main(argv):
    inputfile = ''
    stationsfile = None
    count = None
    try:
        opts, args = getopt.getopt(argv, "hi:s:c:", ["ifile=", "stations", "count"])
    except getopt.GetoptError:
        print('convert.py -i <inputfile>  -s <stationsfile> -c <count>')
        sys.exit(2)
    if not opts:
        print('convert.py -i <inputfile> -s <stationsfile>  -c <count>')
        sys.exit()
    for opt, arg in opts:
        if not opt or opt == '-h':
            print('convert.py -i <inputfile> -s <stationsfile>  -c <count>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-s", "--stations"):
            stationsfile = arg
        elif opt in ("-c", "--count"):
            count = int(arg)

    convert(inputfile, stationsfile, count)


def convert(input_file, stationsfile, count):
    print(" Loading File:"),
    dict = read_excel_to_dict(input_file)
    for key, value in dict.items():

        lines = value

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
                result_dict = {}
                for line in station_sublist:
                    update_or_insert_result_line(base_line, line, result_dict, station)
                insert_nodata_dates(base_line, highest_date, lowest_date, result_dict, station)
                write_dict_to_result(result_dict, result_list)
            print("."),
        print("")
        print("writing output ")
        output_file = "output/" + value[0][0] + "_" + key + ".txt"
        output = open(output_file, 'w')
        for line in result_list:
            output.write("%s\n" % '\t'.join(map(str, line)))
        print(" ")
        print("Done!")


def write_dict_to_result(result_dict, result_list):
    for key in sorted(result_dict.keys()):
        val = result_dict[key]
        print(val)
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


def is_date(string):
    try:
        parse(str(string))
        return True
    except ValueError:
        return False


def read_excel_to_dict(input_file):
    parameters = []
    df = pd.read_excel(input_file)
    station_name = os.path.basename(input_file).split('.')[0][:6].strip()
    #format strin to 6 chars fill empty with '_'
    station_name = '{:_<6}'.format(station_name)
    part_one = False
    part_two = False
    polluent_dict = {}
    for index, row in df.iterrows():
        if row[0] == "Parameter":
            parameters = row[1].split(',')
            for p in parameters:
                polluent_dict[p] = []
        if row[0] == "From Date":
            if not part_one and not part_two:
                part_one = True
            elif part_one and not part_two:
                part_two = True
                part_one = False
            else:
                print("fail")
        if is_date(row[0]) and part_one and not part_two:
            line = [station_name, parameters[0], row[2]]
            for idx, val in enumerate(parameters[:5]):
                if row[2 + idx] != 'None':
                    polluent_dict[val].append([station_name, float(row[2 + idx]), row[0]])
        if is_date(row[0]) and not part_one and part_two:
            line = [station_name, parameters[0], row[2]]
            for idx, val in enumerate(parameters[5:]):
                if row[2 + idx] != 'None':
                    polluent_dict[val].append([station_name, float(row[2 + idx]), row[0]])

    return polluent_dict


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
    date = parse(str(date_field), dayfirst=True)
    return date


if __name__ == "__main__":
    main(sys.argv[1:])
