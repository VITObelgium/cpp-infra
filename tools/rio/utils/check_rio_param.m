% check_rio_param.m
% checking difference in configuration files fortran vs. matlab

clear all;
close all;

tol      = 1.0e-6;
smb      = '\\samba.rma.vito.local\homes\devel\rio\fortran\trunk\';
pol      = 'no2'; 
param    = 'stat_param';  % 'trend', 'spatial_corr', 'stat_param', 'stations', 'grid';

%% Defined configurations in operaiontal RIO model : list of checks
% Define pollutant
switch pol
    case {'pm10','so2','pm25'}
        agg_time = { 'da', '1h' }; % for pm10, so2
    case 'no2'
        agg_time = { '1h', 'm1' }; % for no2
    case 'o3'
        agg_time = { '1h' }; % for o3
    case 'o3s'
        agg_time = { '1h', 'm1', 'm8' }; % for o3s
    otherwise
        error( 'Wrong pollutant...' );
end

% Define gis type
switch pol
    case 'pm10'
        gis_type = { 'CorineID', 'clc06d' }; % for pm10
    case {'no2','o3','o3s','so2'}
        gis_type = { 'CorineID' }; % for no2, o3, o3s, so2
    case 'pm25'
     gis_type = { 'AODBETA' }; % for pm25
    otherwise
        error( 'Wrong pollutant...' );
end

% Define week aggregations
switch pol
    case {'pm10','pm25'}
        wk = { 'weekend', 'week', 'all' }; % for pm10, pm25
    case {'no2','o3','o3s','so2'}
        wk = { 'weekend', 'week' }; % for no2, o3, o3s, so2
    otherwise
        error( 'Wrong pollutant...' );       
end

% Define grid types
switch pol
    case {'pm10','no2','o3','o3s','so2'}
        grid_type = { '4x4', 'belEUROS' }; % pm10,no2,o3,so2
    case 'pm25'
        grid_type = { '4x4' }; %pm25
    otherwise
        error( 'Wrong pollutant...' );       
end
    

%% here we go
trend_str = { 'avg_trend', 'avg_err_trend', 'std_trend', 'std_err_trend' };
trend_par = { 'p_avg', 'p_avg_err', 'p_std', 'p_std_err' };

spcorr_str = { 'p_long', 'p_short' };
spcorr_par = { 'p', 'p_short' };

stat_str   = { 'avg', 'std' };
stat_par   = { 'xx_avg', 'xx_std' };

switch param
    case 'trend'
        for l=1:length(wk)
            for k=1:length(gis_type)
                for j=1:length(agg_time)
                    for i=1:length(trend_str)
                        name = sprintf( '%s_%s_%s_%s_agg_time-%s', trend_str{i}, pol, gis_type{k}, wk{l}, agg_time{j} );
                        a = importdata( fullfile( smb, 'rio_param', param, pol, sprintf( '%s.txt', name ) ) );
                        p = load( fullfile( 'param', pol, param,  sprintf( '%s.mat', name ) ) );
                        df  = abs(a-p.(trend_par{i}));
                        mdf = max(df(:));
                        if mdf > tol,
                            fprintf( '%s: %.8f : NOT OK !!\n', name, mdf );
                        else
                            fprintf( '%s: %.8f : OK.\n', name, mdf );
                        end
                    end
                end
            end
        end
        
    case 'spatial_corr'
        if ~strcmp( pol, 'pm25' )
            gis_type{end+1} = 'no_detr'; % add this guy
        else
            warning( 'no no_detr present for pm25, fix this sometime !!' );
        end
        for k=1:length(gis_type)
            for j=1:length(agg_time)
                for i=1:length(spcorr_str)
                    name = sprintf( '%s_%s_%s_agg_time-%s', spcorr_str{i}, pol, gis_type{k}, agg_time{j} );                    
                    fprintf( '%s', name );
                    a = importdata( fullfile( smb, 'rio_param', param, pol, sprintf( '%s.txt', name ) ) );
                    p = load( fullfile( 'param', pol, param,  sprintf( '%s.mat', name ) ) );
                    df  = abs(a-p.(spcorr_par{i}));
                    mdf = max(df(:));
                    if mdf > tol,
                        fprintf( ' %.8f : NOT OK !!\n', mdf );
                    else
                        fprintf( ' %.8f : OK.\n',  mdf );
                    end
                end
            end
        end

        
    case 'stat_param'
        for l=1:length(wk)
            for j=1:length(agg_time)
                for i=1:length(stat_str)
                    name = sprintf( '%s_%s_%s_agg_time-%s', stat_str{i}, pol, wk{l}, agg_time{j} );
                    a = importdata( fullfile( smb, 'rio_param', param, pol, sprintf( '%s.txt', name ) ) );
                    p = load( fullfile( 'param', pol, param,  sprintf( '%s.mat', name ) ) );
                    df  = abs(a-p.(stat_par{i}));
                    mdf = max(df(:));
                    if mdf > tol,
                        fprintf( '%s: %.8f : NOT OK !!\n', name, mdf );
                    else
                        fprintf( '%s: %.8f : OK.\n', name, mdf );
                    end
                end                
            end
        end
        
        
    case 'stations'
        for i=1:length(gis_type)
            name = sprintf( '%s_stations_info_GIS_%s.txt', pol, gis_type{i} );
            a = importdata( fullfile( smb, 'stations_info', pol, name ) );
            b = importdata( fullfile( 'stations', pol, name ) );
            df  = abs(a.data-b.data);
            mdf = max(df(:));
            if mdf > tol,
                fprintf( '%s: %.8f : NOT OK !!\n', name, mdf );
            else
                fprintf( '%s: %.8f : OK.\n', name, mdf );
            end            
        end
        
      case 'grid'
          for j=1:length(grid_type)
              for i=1:length(gis_type)                  
                  name = sprintf( '%s_grid_%s.txt', gis_type{i}, grid_type{j} );                  
                  b = importdata( fullfile( 'drivers', pol, name ) );
                  if strcmp( grid_type{j}, '4x4' )
                      % in fortran, no 4x4 is present
                    name = sprintf( '%s_grid.txt', gis_type{i} );
                  end
                  a = importdata( fullfile( smb, 'land_use', sprintf( 'land_use_for_%s', pol ), name ) );
                  df  = abs(a.data-b.data);
                  mdf = max(df(:));
                  if mdf > tol,
                      fprintf( '%s: %.8f : NOT OK !!\n', name, mdf );
                  else
                      fprintf( '%s: %.8f : OK.\n', name, mdf );
                  end
              end
          end
        
    otherwise
        error( 'Unknown parameter set : %s' );
end
        