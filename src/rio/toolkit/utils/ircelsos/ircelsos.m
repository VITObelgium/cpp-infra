function ircelsos( varargin )

%FIXME check if timestamps are after or before the hour...

% -- define command line arguments
argopts = [ ...
  struct( 'name', 'output',  'default', '',  'cast', @(x)(x)), ...
  struct( 'name', 'st_list',  'default', '/tools/ircel/1.0/share/dbq_stations.txt',  'cast', @(x)(x)), ...
  struct( 'name', 'help',    'default', false,  'cast', NaN ) ];

% -- Parse and some initial checks!
[ opts, args ] = getopt_cast( argopts, '--', varargin );
if opts.help, print_usage; return; end;

if length(args) == 2
  pol = args{1};
  utc_start = now-str2double(args{2});
  utc_end   = now;
elseif length(args) == 3
  pol = args{1};
  utc_start = datenum(args{2}, 'yyyymmddHHMM');
  utc_end   = datenum(args{3}, 'yyyymmddHHMM');
else
  error('try --help');
end

% -- define proxy settings vor VITO
java.lang.System.setProperty('http.proxyHost', '172.22.1.3');
java.lang.System.setProperty('http.proxyPort','8080');

% -- import station list
st_list = importdata( opts.st_list );

% -- set output
if isempty(opts.output)
  opts.output = sprintf( 'sos_%s_data.txt', pol );
end

fprintf( 'Requested pollutant : %s\n', pol );
fprintf( 'Start time          : %s\n', datestr(utc_start) );
fprintf( 'End time            : %s\n', datestr(utc_end) );

fid = fopen( opts.output, 'wt' );
% -- go
for i_st = 1:length(st_list)
  station   = st_list{i_st};
  station(1:2) = [];
  station = [ 'BET' station ];
  try 
    fprintf( 'Querying %s -> %s...', st_list{i_st}, station );
    [ xx_date, xx_value, st_info ] = query_ircelsos( station, pol, utc_start, utc_end );
    fprintf( 'done.\n' );
  catch Mex1
    try
      station(1:3) = [];
      station = [ 'BEL' station ];
      fprintf( 'failed, trying %s...', station );
      [ xx_date, xx_value, st_info ] = query_ircelsos( station, pol, utc_start, utc_end );
      fprintf( 'done.\n' );
    catch Mex2
      fprintf( 'failed, skipping.\n' );
      continue;
    end
  end
    
  if isempty( xx_date ) || isempty( xx_value ), continue; end;
  
  t1 = floor(min(xx_date));
  t2 = floor(max(xx_date));
  
  tmp_date = transpose( t1:datenum(0,0,0,1,0,0):t2+datenum(0,0,0,23,0,0) );
  tmp_value = nan(size(tmp_date));
  for k=1:length(tmp_date)
    ii = find( abs( xx_date - tmp_date(k) ) < 1.e-6 );
    if any(ii)
      tmp_value(k) = xx_value(ii);
    end
  end
  
  tmp_value = reshape(tmp_value,24,size(tmp_value,1)/24)';
  tmp_date  = reshape(tmp_date,24,size(tmp_date,1)/24)';
  %tmp_agg   = nanmean(tmp_value,2);
  
  tmp_value(isnan(tmp_value)) = -9999;
  
  for l=1:size(tmp_value,1)
    fprintf( fid, '%s %s', st_info.name, datestr(floor(tmp_date(l)),'yyyymmdd') );
    for m=1:24,fprintf( fid, ' %8.1f', tmp_value(l,m) ); end;
    fprintf( fid, '\n' );
  end
     
end
fclose(fid);
fprintf( 'Output in : %s\n', opts.output );
fprintf( 'Done.\n' );


function print_usage
fprintf( 'Usage:\n' );
fprintf( '  ircelsos [options] <pol> <ndays>\n' );
fprintf( '  ircelsos [options] <pol> <date1> <date2>\n' );
fprintf( 'Available options:\n' );
fprintf( '  --help ............. : this message\n' );
fprintf( '  --st_list <file> ... : file with station list\n' );
fprintf( '  --output <file> .... : output filename (default: auto)\n' );
fprintf( 'Remarks:\n' );
fprintf( '  use format yyyymmddHHMM for dates\n' );
