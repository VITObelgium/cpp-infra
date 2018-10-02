%% Interpolate to specified grid location
% The new version, based on libRIO
% (c) libRIO 2010/2011 (bino.maiheu@vito.be)


%clear all
%close all

%% General configuration
pollutant  = 'no2';
agg_time   = 'da';
gis_type   = 'CorineID';
grid_type  = '4x4';
deployment = 'IRCEL';

% note : use hours for 1h agg_time !!
time_step  = datenum(0,0,1);
xx_date    = datenum(2007,01,01):time_step:datenum(2007,12,31);


% Specify location to interpolate to, Retie : 
filename    = 'rio_CorineID_zeebrugge_no2_2009.mat';
point_indic = 0.6810;

point_x     = 70360.0;
point_y     = 227000.0;


%% RIO Inialisation
% configure rio
cnf = rio_init( '.', pollutant, agg_time, gis_type, grid_type );
cnf.deployment = deployment;

% load station information
cnf = rio_loadstationinfo( cnf );

% load the measurements database
cnf = rio_loaddb( cnf );


xx_val = zeros( 1, length( xx_date ) );
xx_err = zeros( 1, length( xx_date ) );

% here we go
for k=1:length(xx_date)
    
    fprintf( 'Handling %s \n', datestr( xx_date(k) ) );
    
    cnf = rio_updatepars( cnf, xx_date(k) );
    
    [st_info_tmp, xx_data] = rio_dblookup( cnf, xx_date(k) );                  
    
    % now interpolate from here to the location of the missing station    
    switch cnf.ipol_mode
        case 'RIO'
            xx_detr = rio_detrend( cnf, st_info_tmp, xx_data );
            C_inv   = rio_covmat( cnf, st_info_tmp );
            [ val, err ]   = rio_krige( cnf, C_inv, st_info_tmp, xx_detr, point_x, point_y );
            [ xx_val(k) xx_err(k) ] = rio_addtrend( cnf, val, err, point_indic );
            
        case 'OrdKrig'
            C_inv   = rio_covmat( cnf, st_info_tmp );
            [ xx_val(k) xx_err(k) ] = rio_krige( cnf, C_inv, st_info, xx_data, point_x, point_y );
            
        otherwise
            error( 'rio_validate:: mode %s not implemented here', cnf.ipol_mode );
            return;
    end        
    
end

% Plot & save
plot( xx_date, xx_val, 'k-' );
hold on;
plot( xx_date, xx_val+xx_err, 'g:' );
plot( xx_date, xx_val-xx_err, 'g:' );
datetick;
hold off;


save( filename, 'xx_date', 'xx_val', 'xx_err', 'point_x', 'point_y', 'point_indic' );

    
