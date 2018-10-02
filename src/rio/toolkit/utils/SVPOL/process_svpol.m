%PROCESS_SVPOL
%
% process_svpol( svpol_file, pollutation, station_file )
%
% Processes the SVPOL file with the 48 half-hourly values (this time in floating point)
% to the RIO matlab input files. The routine writes the xx_val_1h.mat,
% xx_days.mat and xx_val_max.mat files based upon the station file provide
% to the routine.
%
% This routine was created as a replacement for the dbquery tool since
% Frans decided to switch to floating point numbers for the half hourly
% values to start from.
%
% Averaging rules:
%  - Hourly values from the half-hourly values are computed if *both*
%    half-hourly values are valid, otherwise the resulting value is -9999
%
% See also read_svpol 
%
% Author: Bino Maiheu (c) 2012 bino.maiheu@vito.be

function process_svpol( svpol_file, pollutant, station_file, varargin )

global st

% Compute station ID for the given station_file
st = importdata( station_file );

% fname = 'pm10_2000_2011_calibrated_decimals.dat';
[ st_name xx_date xx_vals48 ] = read_svpol( svpol_file, pollutant );

st_id = cell2mat(cellfun( @getid, st_name, 'UniformOutput', false ));

% Kill the unknown stations
xx_vals48( st_id == -1, : ) = [];
xx_date( st_id == -1, : )   = [];
st_name( st_id == -1 )      = [];
st_id( st_id == -1 )        = [];

% Interleave the first half and second half of the hour to compute the mean
xx_vals48( xx_vals48 < 0 ) = NaN;
if strcmp( pollutant, 'bc' )
  fprintf( 'Converting bc to µg/m3... divide by 100\n' );
  xx_vals48 = xx_vals48/100;
end

xx_val48_1 = xx_vals48(:,1:2:end);
xx_val48_2 = xx_vals48(:,2:2:end);


xx_val_1h  = .5*( xx_val48_1 + xx_val48_2 ); % make use of NaN -> if one value is NaN -> average is NaN
xx_val_1h( isnan(xx_val_1h) ) = -9999;   % reset missing values

% compute the daily mean & max values based upon the hourly values
% these are not rounded here yet...
if strcmp( pollutant, 'bc' )
  xx_val_m1 = max( xx_val_1h, [], 2 );
  xx_val_da = eu_mean( xx_val_1h );
else
  xx_val_m1 = round( max( xx_val_1h, [], 2 ) );
  xx_val_da = round( eu_mean( xx_val_1h ) );
end
%xx_val_m8 = round( nanmax( [ nanmean(xx_vals48(:,1:16), 2 )  nanmean(xx_vals48(:,17:32), 2 )   nanmean(xx_vals48(:,33:48), 2 ) ], [], 2 ) );

disp( 'M8 IS STILL TODO --> glijdend gemiddelde voor elk uur van de laaste 8 uurswaarden, dus uur 1 neemt 7 waarden van vorige dag mee...' );
xx_val_m8 = -9999 * ones( size(xx_val_m1 ) );
xx_val_m1( isnan( xx_val_m1 ) ) = -9999;
xx_val_da( isnan( xx_val_m1 ) ) = -9999;

% Round also xx_val_1h to nearest integer
if strcmp( pollutant, 'bc' )
  xx_val_1h  = horzcat( st_id, xx_val_1h );
  xx_val_max = horzcat( st_id, xx_val_m1, xx_val_m8, xx_val_da );
else
  xx_val_1h  = horzcat( st_id, round( xx_val_1h ) );
  xx_val_max = horzcat( st_id, xx_val_m1, xx_val_m8, xx_val_da );
end

save( sprintf( '%s_days.mat',    pollutant ), 'xx_date' );
save( sprintf( '%s_data_1h.mat', pollutant ), 'xx_val_1h' );
save( sprintf( '%s_data_max.mat', pollutant ), 'xx_val_max' );


function id = getid( x )
global st

i = find( strcmp( st.textdata(:,2), x ) );
if isempty( i )
    id = -1; % id not found
else
    id = str2double(st.textdata{ i, 1 });
end


%helper function to compute the EU mean values...
function avg = eu_mean( x )
avg = -9999*ones(size(x,1),1);
for k=1:size(x,1)
    arr = x(k,:);
    if numel( find( arr < 0 ) ) > 6 % need at least 75 % valid hourly values
        avg(k) = -9999;
    else        
        avg(k) = mean( arr( arr > 0 ) );
    end
end
