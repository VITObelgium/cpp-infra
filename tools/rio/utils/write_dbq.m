% writes a dbq file from the matlab .mat files...
% Bino Maiheu
% N78C0

clear variables;
close all;

fname   = 'pm10_data_rio_float.txt';
pol     = 'pm10';
ver     = 'v2016';
st_info = importdata( 'stations/v2016/pm10/pm10_stations_info_GIS_clc06d.txt' );

%% here we go
t_days = load( fullfile( 'data', ver, pol, sprintf( '%s_days.mat', pol ) ) );
t_hour = load( fullfile( 'data', ver, pol, sprintf( '%s_data_1h.mat', pol ) ) );
t_max  = load( fullfile( 'data', ver, pol, sprintf( '%s_data_max.mat', pol ) ) );

% get list of stations matching the 
st_idx = t_max.xx_val_max(:,1);
st_info.textdata(st_idx+1,2);
st_list = st_info.textdata(st_idx+1,2); % note this assumes sequential behaviour...

fid = fopen( fname, 'wt' );
for k=1:size(t_days.xx_date)
%  export = [ round(100.*t_max.xx_val_max(k,2:4)) round(100.*t_hour.xx_val_1h(k,2:25)) ];
  export = [ round(t_max.xx_val_max(k,2:4)) round(t_hour.xx_val_1h(k,2:25)) ];
  export( export < 0 ) = -9999;
%  fprintf( fid, '%6s %6s %6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n', ...
%    st_list{k}, datestr( t_days.xx_date(k), 'yyyymmdd' ), export ); 

if ( t_days.xx_date(k) >= datenum( 2014,1,1) && t_days.xx_date(k) < datenum( 2015,1,1) )
    fprintf( fid, '%6s %6s %10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f\n', ...
        st_list{k}, datestr( t_days.xx_date(k), 'yyyymmdd' ), export );
end
end
fclose(fid);
