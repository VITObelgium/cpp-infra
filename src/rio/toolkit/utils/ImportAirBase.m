function ImportAirBase( path, country, pol )
% airbaseImport( path, country, pol, type )

% some parameters
airbase_codes_file = fullfile( path, 'airbase_v5_codes.txt' );
type               = 'hour'; % select hourly data & compute daily stats ourselves

% Import airbase codes...
fprintf( 'Importing airbase codes from %s\n', airbase_codes_file );
fid = fopen( airbase_codes_file, 'r' );
tmp = textscan(fid, '%d%s%s%s', 'Delimiter', ';', 'headerlines', 1 );
fclose( fid );
idx = find( strcmpi( tmp{2}, pol ) );
if idx < 1
    error( 'cannot find airbase code for %s', pol );
end
airbase_code = tmp{1}( idx );
fprintf( 'Airbase code for %s = %d\n', pol, airbase_code );

% Build filelist...
flist    = dir( sprintf( '%s/%s*', fullfile( path, country, '' ), country ) );

% Some arrays to store the harvest
station_id   = 0;
station_list = {};
xx_stations  = [];
xx_date      = [];
xx_val       = [];
xx_max       = [];

for i=1:length( flist )        
    info = airbaseFileInfo( flist(i).name );     
    
    % Check component & type
    if ( info.component_code ~= airbase_code ) || ...
            ~strcmpi( info.data_type, type )        
        continue;
    else
        fprintf( '%s contains %s %s data, parsing...\n', flist(i).name, type, pol );
    end
    
    fid = fopen( fullfile( path, country, flist(i).name ) );
    switch( info.data_type )
        case { 'hour' }
            % Parse this station
            tmp = textscan( fid, strcat( '%s', repmat( '%f%d', [1 24] ) ), ...
                'Delimiter', '\t' );
            
            tmp_date = datenum( tmp{1} );
            tmp_vals = cell2mat( { tmp{ :,2:2:48} } );
            tmp_qual = cell2mat( { tmp{ :,3:2:49} } );

            % apply quality checks
            tmp_vals( tmp_qual < 1 ) = -9999;
            
            tmp_max  = calc_daily( tmp_vals );
                       
        otherwise
            error( 'data type %s is not supported', info.data_type );
    end    
    fclose(fid);
    
    % Check if we have the station already ?
    if any( strcmp( station_list, info.station_eu_code ) ) 
       fprintf( '+++ warning : we have already station %s \n', info.station_eu_code );
    else        
        station_list{end+1} = info.station_eu_code;
    end
    
    % Append arrays...
    xx_stations = [ xx_stations; repmat( info.station_eu_code, size( tmp_vals, 1 ), 1 ) ];
    xx_date     = [ xx_date;  datestr( tmp_date, 'yyyymmdd' ) ];
    xx_max      = [ xx_max; tmp_max ];
    xx_val      = [ xx_val; tmp_vals ];    
end

% Now store the data in RIO dbq format...
fid = fopen( sprintf( 'RIO_AIRBASE_%s_%s.dbq', upper( country ), upper( pol ) ), 'wt' );
for k=1:size(xx_val,1)    
    fprintf( fid, '%6s %6s ', xx_stations( k, [1:2 4:7] ), xx_date(k,:) );
    fprintf( fid, '%6d%6d%6d', round( xx_max(k,:) ) );
    fprintf( fid, '%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d', round( xx_val(k,:) ) );
    fprintf( fid, '\n' );
end
fclose( fid );




% stations file
fid = fopen( sprintf( '%s_stations_1h.txt', pol ), 'wt' );
fprintf( fid, 'ID;NAME\n' );
for i=1:length( station_list ); fprintf( fid, '%d;%s\n', i, station_list{i} ); end;
fclose(fid);
    


function s = airbaseFileInfo( filename )
% retrieves some info on the filename, returns a structure
s    = struct();

% split in 3 parts   
part = regexp( filename, '\.', 'split');
    
s.station_eu_code           = part{1}(1:7);
s.component_code            = str2double( part{1}(8:12) );
s.measurement_eu_group_code = part{1}(13:17);
s.data_type                 = part{1}(18:end);

% parse the start & end dates for the timeseries
s.start_date                = airbaseConvertDate( part{2} );
s.end_date                  = airbaseConvertDate( part{3} );

function d = airbaseConvertDate( str )
part = regexp( str, '-', 'split');
d = datenum( str2double(part{3}), str2double(part{2}), str2double(part{1}) );

function daily = calc_daily( hourly )

h1 = hourly(:,1:8);
h2 = hourly(:,9:16);
h3 = hourly(:,17:24);

daily = zeros( size(hourly,1), 3 );

for i=1:size( hourly,1 )
   
    my_h  = hourly(i,  hourly(i, :) > 0 );
    
    my_h1 = h1(i, h1(i,:) > 0 );
    my_h2 = h2(i, h2(i,:) > 0 );
    my_h3 = h3(i, h3(i,:) > 0 );
    
    my_m8 = [ mean( my_h1 ) mean( my_h2 ) mean( my_h3 ) ];
    
    if ~isempty( my_h )
        daily(i,1) = max( my_h );
    else
        daily(i,1) = -9999;
    end
    
    if ~isempty( my_m8( ~isnan( my_m8 ) ) )
        daily(i,2) = max( my_m8( ~isnan( my_m8 ) ) );
    else 
        daily(i,2) = -9999;
    end
    
    if ~isempty( my_h )
        daily(i,3) = mean( my_h );
    else 
        daily(i,3) = -9999;
    end
end