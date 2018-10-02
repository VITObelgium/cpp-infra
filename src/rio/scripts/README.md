scripts folder

convert_param.py - this script converts the old rio parameter structure to the new xml version

convert.py - this script converts the CZ input file to the rio input format.

        output:
        STATION DATUM M1 M8 DA H1 H2 H3 H4 .... H24
        with
        	M1 : dag maximum
        	M8 : 8-uur glijdend maximum
        	DA : daggemiddelde
        	H1 .. H24 : 24 uurlijkse waarden
            
        params:
        -i input file
        -o output file
        -s stations file (to limit which stations to prcoess)
        -c count (to limit the count of measurements to process)

convert_india.py - this script converts the indian input files to the rio input format.

        output:
        STATION DATUM M1 M8 DA H1 H2 H3 H4 .... H24
        with
        	M1 : dag maximum
        	M8 : 8-uur glijdend maximum
        	DA : daggemiddelde
        	H1 .. H24 : 24 uurlijkse waarden
        1 file per station per parameter
        
        usage:
        python convert_india -i "input file.xlsx"
        output needs an "output" folder in the script directory.