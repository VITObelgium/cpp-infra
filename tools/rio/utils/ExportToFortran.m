%% ExportToFortran
% The new version, based on libRIO
% (c) libRIO 2010/2011 (bino.maiheu@vito.be)
%
% This script reads in all the rio parameter files and creates an export to
% load in the fortran version of RIO. It also gathers the land_use and
% stations files

%clear all
%close all

%% General configuration
pollutant        = 'bc';
agg_time         = 'da';
gis_type         = 'clc06d';
grid_type        = '4x4';
base             = '.';
outputdir        = './export';
export_grid      = true;
export_stations  = true;

% this is true if we are using the new folder structure, in which case
% we need to set the version as well
new_struct       = true;
ver              = 'v3.6';



%% Here we go
% create output folder structure
trend_dir = fullfile( outputdir, 'param', 'trend', pollutant, '' );
spcor_dir = fullfile( outputdir, 'param', 'spatial_corr', pollutant, '' );
statp_dir = fullfile( outputdir, 'param', 'stat_param', pollutant, '' );
stati_dir = fullfile( outputdir, 'stations', pollutant, '' );
landu_dir = fullfile( outputdir, 'drivers', pollutant , '' );
if ~exist( trend_dir, 'file' ), mkdir( trend_dir); end;
if ~exist( spcor_dir, 'file' ), mkdir( spcor_dir); end;
if ~exist( statp_dir, 'file' ), mkdir( statp_dir); end;
if ~exist( stati_dir, 'file' ), mkdir( stati_dir); end;
if ~exist( landu_dir, 'file' ), mkdir( landu_dir); end;

% export grid
if export_grid
    fprintf( 'Exporting grid...\n' );
    grid_file = fullfile( base, 'drivers', ver, pollutant, ...
      sprintf( '%s_grid_%s.txt', gis_type, grid_type ) );    
    copyfile( grid_file, landu_dir );
end

% export stations
if export_stations
    fprintf( 'Exporting stations...\n' );
    if new_struct
      station_file = fullfile( base, 'stations', ver, pollutant, ...
        sprintf( '%s_stations_info_GIS_%s.txt', pollutant, gis_type ) );
    else
      station_file = fullfile( base, 'stations', pollutant, ...
        sprintf( '%s_stations_info_GIS_%s.txt', pollutant, gis_type ) );
    end
    copyfile( station_file, stati_dir );
end
   
% export the trend functions
fprintf( 'Exporting rio parameters functions...\n' );
for param = { 'trend', 'spatial_corr', 'stat_param' }    
    
    if strcmp( param, 'spatial_corr' )        
        range_list_1 = {'long', 'short'};
        range_list_2 = {''};
    elseif strcmp( param, 'trend')        
        range_list_1 = {'week', 'weekend', 'all'}; 
        range_list_2 = {'avg', 'std', 'avg_err', 'std_err' };
    elseif strcmp( param, 'stat_param')        
        range_list_1 = {'week', 'weekend', 'all'};
        range_list_2 = {'avg' , 'std'};
    end
            
    for range_1 = range_list_1
        for range_2 = range_list_2
            
            fprintf( 'Exporting %s : %s %s\n', char(param), char(range_1), char(range_2) );
            
            switch( char( param ) )
                case 'trend'
                  if new_struct
                    infile = fullfile( base, 'param', ver, pollutant, char(param), ...
                      sprintf( '%s_trend_%s_%s_%s_agg_time-%s.mat',  char(range_2), ...
                      pollutant, gis_type, char(range_1), agg_time ) );
                  else
                    infile = fullfile( base, 'rio_param', char(param), pollutant, ...
                      sprintf( '%s_trend_%s_%s_%s_agg_time-%s.mat',  char(range_2), ...
                      pollutant, gis_type, char(range_1), agg_time ) );
                  end
                  outfile = fullfile( trend_dir, ...
                    sprintf( '%s_trend_%s_%s_%s_agg_time-%s.txt',  char(range_2), ...
                    pollutant, gis_type, char(range_1), agg_time ) );
                    
              case 'spatial_corr'
                if new_struct
                  infile = fullfile( base, 'param', ver, pollutant, 'spatial_corr', ...
                    sprintf( 'p_%s_%s_%s_agg_time-%s.mat',  char(range_1), ...
                    pollutant, gis_type, agg_time ) );
                else
                  infile = fullfile( base, 'rio_param', 'spatial_corr', pollutant, ...
                    sprintf( 'p_%s_%s_%s_agg_time-%s.mat',  char(range_1), ...
                    pollutant, gis_type, agg_time ) );
                end
                    outfile = fullfile( spcor_dir, ...
                        sprintf( 'p_%s_%s_%s_agg_time-%s.txt',  char(range_1), ...
                        pollutant, gis_type, agg_time ) );                
                
                case 'stat_param'
                  if new_struct
                    infile = fullfile( base, 'param', ver, pollutant, 'stat_param', ...
                        sprintf( '%s_%s_%s_agg_time-%s.mat', char(range_2), ...
                        pollutant, char(range_1), agg_time ) );
                  else
                    infile = fullfile( base, 'rio_param', 'stat_param', pollutant, ...
                        sprintf( '%s_%s_%s_agg_time-%s.mat', char(range_2), ...
                        pollutant, char(range_1), agg_time ) );
                  end
                    outfile = fullfile( statp_dir, ...
                        sprintf( '%s_%s_%s_agg_time-%s.txt', char(range_2), ...
                        pollutant, char(range_1), agg_time ) );
                    
            end
            
            % load the file
            p_tmp = load( infile );
            
            % select the right variable
            switch( char( param ) )
                case 'trend'
                    if strcmp(range_2, 'avg')
                        p = p_tmp.p_avg;
                    elseif strcmp(range_2, 'std')
                        p = p_tmp.p_std;
                    elseif strcmp(range_2, 'avg_err')
                        p = p_tmp.p_avg_err;
                    elseif strcmp(range_2, 'std_err')
                        p = p_tmp.p_std_err;                        
                    end
                case 'spatial_corr'
                    if strcmp(range_1, 'long')
                        p = p_tmp.p;
                    elseif strcmp(range_1, 'short')
                        p = p_tmp.p_short;
                    end       
                case 'stat_param'
                    if strcmp(range_2, 'avg')
                        p = p_tmp.xx_avg;
                    elseif strcmp(range_2, 'std')
                        p = p_tmp.xx_std;
                    end
            end
            
            % export to ascii
            fprintf( 'writing %s -> %s\n', infile, outfile );
            save( outfile, 'p', '-ascii');
            
            clear p p_tmp;
        end
    end
end




