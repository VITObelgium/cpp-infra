function [ st_name xx_date xx_vals48 ] = read_svpol( fname, pol )

switch lower(pol)
  case 'pm10', saroad_req = '81102';
  case 'o3',   saroad_req = '44201';
  case 'no2',  saroad_req = '42602';
  case 'pm25', saroad_req = '81104';
  case 'so2',  saroad_req = '42401';
  case 'co',   saroad_req = '42101';
  case 'bzn',  saroad_req = '45201';
  case 'bc',   saroad_req = '16111';
  otherwise
    error( 'pollutant not supported' );
end

fid = fopen( fname, 'r' );
tmp = textscan( fid, '%s%s%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f' );
fclose(fid);

% extract the stations and the saroad codes
st_name = cellfun( @(x) x(1:6), tmp{:,1}, 'UniformOutput', false );
saroad  = cellfun( @(x) x(7:end), tmp{:,1}, 'UniformOutput', false );



% extract dates
fprintf( 'Extracting dates...\n' );
xx_date  = cellfun( @(x) datenum( str2double(x(2:5)), str2double(x(6:7)), str2double(x(8:9) ) ), tmp{:,2}, 'UniformOutput', false );
xx_date  = cell2mat(xx_date);

% extract values
fprintf( 'Extracting values...\n' );
xx_vals48 = horzcat( tmp{3:end} );


% 
% if any( ~strcmp( saroad, saroad_req ) )
%     error( 'Pollutant present in file other than requested %s (%s)', pol, saroad_req );
% else
%     fprintf( 'Pollutant check succeeded\n' );
% end

% select
fprintf( 'Select pollutant saroad code %s...\n', saroad_req );
idx = find( strcmpi( saroad, saroad_req ) );
xx_date = xx_date(idx);
st_name = st_name(idx);
xx_vals48 = xx_vals48(idx,:);

fprintf( 'Done...\n' );


