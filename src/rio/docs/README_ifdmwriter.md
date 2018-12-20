RIO ifdmwriter format specification
===================================

This file contains the discription of the ifdmwriter binary output file format for RIO

* Author: Bino Maiheu
* Contact : bino.maiheu@vito.be

## Header format specification

Here we provide an over view of the header format for the 


| variable | C type  | Fortan type | offset | explanation                                                              |
|----------|---------|-------------|--------|--------------------------------------------------------------------------|
| nbytes   |   int32 |   integer*4 |      0 | number of bytes in the header of the file                                |
| version  |   int32 |   integer*4 |      4 | version of this format                                                   |
| nt       |   int32 |   integer*4 |      8 | number of timesteps in the file                                          |
| nx       |   int32 |   integer*4 |     12 | number of columns (x-direction) in the RIO raster                        |
| ny       |   int32 |   integer*4 |     16 | number of rows (y-direction) in the RIO raster                           |
| xul      |   float |      real*4 |     20 | geographical x coordinate of upper left edge of raster (edge not centre) |
| yul      |   float |      real*4 |     24 | geographical y coordinate of upper left edge of raster (edge not centre) |



## Example read code
