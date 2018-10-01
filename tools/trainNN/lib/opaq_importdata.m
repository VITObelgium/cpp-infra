function opaq_importdata

% TODO Now output to HDF5 file & later on also the training sample... output to
%      hdf5 !!

%  -- configuration
st_info      = opaq_readstations( 'etc/network.xml' );
meteo_path   = '//fs.marvin.vito.local/storage/projects/N78C0/OVL/ECMWF/db';
meteo_res    = 3;
meteo_prefix = 'BEL';

%st_info      = opaq_readstations( 'etc/network_old_meteo.xml' );
%meteo_path   = '//fs.marvin.vito.local/storage/projects/N78C0/OVL/ECMWF_lowres/2011';
%meteo_res    = 6;
%meteo_prefix = 'ECMWF';

npars        = 32;
hrs          = datenum(0,0,0,0,0,0):datenum(0,0,0,meteo_res,0,0):datenum(0,0,0,21,0,0);
hrs24        = datenum(0,0,0,0,0,0):datenum(0,0,0,1,0,0):datenum(0,0,0,23,0,0);  % timestamp after te hour !!
import_meteo = false;
import_obs   = true;

% -- prepare the observation databas from the raw files,
%    use RIO format since we have to predict also the max1h/max8h
%    values...
if import_obs,
    db = struct();
    fprintf( 'Importing O3 measurements...\n' );
    db.o3   = importdata( 'data/dbq_20160323_RIO_O3_All.dat' );
    fprintf( 'Importing NO2 measurements...\n' );
    db.no2  = importdata( 'data/dbq_20160323_RIO_NO2_All.dat' );
    fprintf( 'Importing PM10 measurements...\n' );
    db.pm10 = importdata( 'data/dbq_20160323_RIO_PM10_All.dat' );
    fprintf( 'Importing PM25 measurements...\n' );
    db.pm25 = importdata( 'data/dbq_20160323_RIO_PM25_All.dat' );
end

% -- looping over the stations
for i=1:length(st_info)
   
    fprintf( 'Handling station %s...\n', st_info(i).name );
        
    % -- importing measurements & target values
    if import_obs,
        obs_output = sprintf( '%s.mat', st_info(i).name );
        if exist( obs_output, 'file' )
            fprintf( 'Already have this station file, skipping...\n' );
            continue;
        end        
        
        xx_obs = [];
        
        for j=1:length(st_info(i).pollutants)
           pol_name = st_info(i).pollutants{j};
           if isfield( db, pol_name )
               
               % lookup the station in this file
               idx = find( strcmp( db.(pol_name).textdata, st_info(i).name ) );
               
               xx_days   = datenum( num2str( db.( pol_name ).data(idx,1) ), 'yyyymmdd' );
               xx_obs.( pol_name ).xx_max1h  = db.( pol_name ).data(idx,2);
               xx_obs.( pol_name ).xx_max8h  = db.( pol_name ).data(idx,3);
               xx_obs.( pol_name ).xx_dayavg = db.( pol_name ).data(idx,4);
                
               xx_hours = repmat( xx_days, 1, length(hrs24) ) + repmat( hrs24, size(xx_days,1), 1 );
               xx_value = db.( pol_name ).data(idx,5:28);
               
               xx_obs.( pol_name ).xx_hours = reshape( xx_hours', numel(xx_hours), 1 );
               xx_obs.( pol_name ).xx_value = reshape( xx_value', numel(xx_value), 1 );
               xx_obs.( pol_name ).xx_days  = xx_days;
               
               
           else
               fprintf( '+++ we do not have this pollutant loaded : %s...\n', pol_name );
               continue;
           end
        end
        
        xx_info = st_info(i);
        save( obs_output, 'xx_obs', 'xx_info' );
        
    end
    
    % -- importing meteo data if requested
    if import_meteo        
        meteo_output = sprintf( 'METEO_%s.mat', st_info(i).meteo );
        if exist( meteo_output, 'file' )
            fprintf( 'Already have this meteo file, skipping...\n' );
            continue;
        end
        xx_meteo = [];
        
        for j=1:npars            
            fname = fullfile( meteo_path, sprintf( '%s_%s_P%02d.txt', meteo_prefix, st_info(i).meteo, j ) );
            fprintf( '  loading %s\n', fname );
            xx_data = load( fname );
            
            %-- convert data
            xx_date   = datenum( num2str( xx_data(:,1) ), 'yyyymmdd' );            
            xx_date = repmat( xx_date, 1, length(hrs) ) + repmat( hrs, size(xx_date,1), 1 );
            xx_data = xx_data(:,2:end);
            
            % re order the data array's
            xx_date = reshape( xx_date', numel(xx_date), 1 );
            xx_data = reshape( xx_data', numel(xx_date), 1 );
            
            
            if j==1
                xx_meteo = [ xx_date, xx_data ];
            else
                
                % TODO check if arrays equal size..
                xx_meteo(:,end+1) = xx_data;
            end
            
        end
        % export the meteo
        fprintf( '  saving %s\n', meteo_output );
        save( meteo_output, 'xx_meteo' );
    end
end