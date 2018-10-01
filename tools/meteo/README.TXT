OPAQ Meteo tools
Bino Maiheu, (c) VITO 2014

This repository contains a number of tools to deal with meteo data in the different OPAQ models


The main tools are 

opaq_import_fnl  : imports the FNL netCDF files into the OPAQ databases
opaq_import_gfs  : idem for the GFS forcasts

1. scripts 

	This subfolder contains an number of retrieval scripts for the GFS and FNL meteo datasets and conversion
	to netcdf. 
	
	
2. src
   Some helper functions, including the opaq_extract_meteo which extracts the meteo from the netCFD files