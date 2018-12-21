This README file describes the RIO v5.0 model configuration

Author: Bino Maiheu
Contact: bino.maiheu@vito.be

## Timestamp conventions in RIO

Here we shed some light on the timestamp conventions used in the RIO model. Internally for programming ease, RIO works with the convention : 

* of using the normal time indications with hours going from 00 to 23 
* using timestamps which apply at the beginning of the interval. 

This means that internally when RIO runs the main loop, the timestepping for one day at hourly intervals is done from 00:00 to 23:00, and these timestamps that are requested from the different datahandlers. 

### Specifying the time ranges
Usually in AQ however, it is custom to provide the the measurements with a timestamp after the hour, therefore the user is able to request the correct interpolation timerange and specify to what side of the interval the requested timestamps apply. This is done with the -t switch, where 0 indicates before the hour and 1 after the hour : 

 rio \[other_options...\] -a 1h -t 0 2017-01-01 2017-01-02

will interpolate the hourly values starting from the measurement for the first hour of 2017-01-01 (i.e. the hourly average from 2017-01-01 00:00 to 2017-01-01 01:00) to the first hour of 2017-01-02, whereas : 

 rio \[other_options...\] -a 1h -t 1 2017-01-01 2017-01-02 

would interpolate the hourly values from the last hour of 2016-12-31 (, i.e. the hourly averaged concentration from 2016-12-31 23:00:00 to 2017-01-01 00:00:00) to the last hour of 2017-01-01, i.e. (from 2017-01-01 23:00 to 2017-01-02 00:00)

To interpolate the 24 hourly values of 2017-01-01 would be then : 

 rio \[other_options...\] -a 1h -t 1 "2017-01-01 01:00:00" 2017-01-02 

or
 
 rio \[other_options...\] -a 1h -t 0 2017-01-01 "2017-01-01 23:00:00"


Note that :
* the -t switch is only applicable for the sub-day time aggregations (hourly values) !
* when given the timestamps at the command line the format is "yyyy-mm-dd \[HH:MM:SS\]", so when specifying hours, the user will need to add quotes

### Conventions in the outputhandlers

Each output handler uses it's own convention for writing output, and can therefore be adjusted as such. Some are fixed, some, such as the ifdmwriter outputfhandler can be configured and the tmode is stored. Note that this tmode in the outputhandler can differ from the tmode which was specified by the user for giving the interval ! See the documentation for each of the datahandlers separately...
