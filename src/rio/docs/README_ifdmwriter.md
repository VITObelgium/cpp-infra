RIO ifdmwriter format specification
===================================

This file contains the discription of the ifdmwriter binary output file format for RIO.

* Author  : Bino Maiheu
* Contact : bino.maiheu@vito.be

## Configuration 

The ifdmwriter makes a new type of output handler available which dumps the interpolated fields per pollutant in a single 
unformatted flat binary file which can easily be read into Fortran (IFDM). In the master RIO xml setup file, the user is
required to specify a <handler> tag of class "ifdmwriter". And specifiying a typical output location for the file, together
with a griddefinition for each of the grids the user wants to make available. In this grid definition, the grid layout should
be specified in the attributes and a mapping file should be given, linking each RIO gridcell to its respective row and column
in the rectangular raster grid (which the ifdmwriter dumps), much in the same way as the aps output format. 

See below for an example of such a specification :

```xml
 <output>    
    <handler name="ifdm" class="ifdmwriter">		
	  <location>rio_%pol%_%ipol%_%grid%-%start_time%-%end_time%.bin</location>		
		<griddef grid="4x4" epsg="31360" missing="-9999"
		         nx="69" ny="57" dx="4000." dy="4000." xul="22000." yul="248000.">
			%base%/grids/rio_4x4_grid.map
		</griddef>		
		<griddef grid="4x4e1" epsg="31360" missing="-9999"
		         nx="71" ny="59" dx="4000." dy="4000." xul="18000." yul="252000.">
			%base%/grids/rio_4x4e1_grid.map
		</griddef>		
	 </handler>
 </output>
 ```

The example above will make the '-o ifdm' output option available to the user for the 4x4 and 4x4e1 grids. 

## Header format specification

Here we provide an over view of the header format for the binary RIO outputfile, we're not too concerned with storage space
allocation in this first version, so just dumping 32 bit float & ints

| Variable |  C type  |  Fortan type | offset | explanation                                                              |
|----------|----------|--------------|--------|--------------------------------------------------------------------------|
| nbytes   |    int32 |    integer*4 |      0 | number of bytes in the header of the file                                |
| version  |    int32 |    integer*4 |      4 | version of this format                                                   |
| nt       |    int32 |    integer*4 |      8 | number of timesteps in the file                                          |
| nx       |    int32 |    integer*4 |     12 | number of columns (x-direction) in the RIO raster                        |
| ny       |    int32 |    integer*4 |     16 | number of rows (y-direction) in the RIO raster                           |
| xul      |    float |       real*4 |     20 | geographical x coordinate of upper left edge of raster (edge not centre) |
| yul      |    float |       real*4 |     24 | geographical y coordinate of upper left edge of raster (edge not centre) |
| dx       |    float |       real*4 |     28 | grid cell size in x direction                                            |
| dy       |    float |       real*4 |     32 | grid cell size in y direction                                            |
| epsg     |    int32 |    integer*4 |     36 | EPSG code number for spatial reference frame                             |
| t0_year  |    int32 |    integer*4 |     40 | year of first timestep in the file                                       |
| t0_month |    int32 |    integer*4 |     44 | month of first timestep in the file                                      |
| t0_day   |    int32 |    integer*4 |     48 | day of first timestep in the file                                        |
| t0_hour  |    int32 |    integer*4 |     52 | hour of first timestep in the file                                       |
| t0_min   |    int32 |    integer*4 |     56 | min of first timestep in the file                                        |
| t0_sec   |    int32 |    integer*4 |     60 | sec of first timestep in the file                                        |
| tmode    |    int32 |    integer*4 |     64 | mode for timestamp, 0: before the hour, 1: after the hour (default)      |
| dt       |    int32 |    integer*4 |     68 | timestep in seconds, 3600 for 1h, 86400 for da,m1,m8                     |
| missing  |    int32 |    integer*4 |     72 | missing value                                                            |
| pol      |  char[4] |  character*4 |     76 | pollutant name                                                           |
| aggr     |  char[4] |  character*4 |     80 | aggregation time : 1h, da, m1, m8, ...                                   |
| class    | char[10] | character*10 |     84 | interpolation model class                                                |
| ipol     | char[10] | character*10 |     94 | interpolation model                                                      |
| conf     | char[10] | character*10 |    104 | RIO configuration string                                                 |
| author   | char[30] | character*30 |    114 | Responsible author for RIO configuration                                 |
| email    | char[30] | character*30 |    144 | Email contact adress                                                     |

Note that the offsets are 0-based, so in Fortran, don't forget to add a 1 in the read(pos=XX) statement. So for example just
reading the pollutant would be : 

```Fortran
  read(unit=1,pos=76+1) pol
  write(*,*)'pollutant       : ', trim(pol)
```

The acutal data is ordered per timestep and then for the rasters in Fortran column-major order, meaning that the fastest
changing index is the row index (y-coordinate), starting from the top left of the raster. 

## Configuration options

Below we specify a number of configuration flags which are included in the header
* *tmode* : contains the timestamp convention, indicating whether the timestamp t0 applies to the beginning or end of the measurmeent interval. 
            The value is only defined for interpolations with a timeresolution below 1 day, otherwise the value is -1 (i.e. n/a)
            tmode can be 0: the t0 timestamp is before the hour to which the concentration value applies, or 1 : the timestamp is after the hour
            to which the concentration value applies (default). 
* *t0_hour* : contains the hour for the concentration value. First of all see the note above regarding tmode and secondly, the hour runs from 0 to 23,                  always, so we are *not* using the 1 -> 24 h convention 

## Example read code

In Fortran, these files can be read easily using a read statement on an unformated stream. Here is some basic example code :

```Fortran
program ifdm_input

    integer*4 :: nbytes, version
    integer*4 :: nt,nx,ny
    integer*4 :: t0_year, t0_month, t0_day, t0_hour, t0_min, t0_sec, tmode, dt
    real*4    :: xul, yul, dx, dy, xc, yc
    integer*4 :: epsg, missing
    character*4 :: pol, aggr
    character*10 :: ipol_class, conf
    character*30 :: author, email

    real*4, allocatable :: conc(:,:)

    open(unit=1, file='rio_output_file.bin', access='stream', action='read', status='old', form='unformatted')
    read(unit=1) nbytes, version
    read(unit=1) nt, nx, ny, xul, yul, dx, dy, epsg, t0_year, t0_month, t0_day, t0_hour, t0_min, t0_sec, tmode, dt, missing
    read(unit=1) pol, aggr, ipol_class, conf, author, email

    do t=1,nt
      
        read(unit=1) conc
        do i=1,ny ! loop over rows
            do j=1,nx ! loop over columns
            write(*,'(F10.2)', advance="no") conc(i,j)
        enddo
        write(*,*)
    enddo

end program ifdm_input
```
