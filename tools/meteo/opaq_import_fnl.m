%OPAQ_IMPORT_FNL Import FNL data into OPAQ (OVL)
%
% Usage:
%   opaq_import_fnl [options] LON1,LAT1;LON2,LAT2;LON3,LAT3;... start_date end_date
%
% Available options:
%   --help ................ : this message
%   --domain <name> ....... : name for the domain (def. china)
%   --repo <path> ......... : path to FNL data (excl year), (def. fnl)
%   --tzone <shift> ....... : conversion from UTC to local time zone
%                             default 0 : no conversion. for conversion to
%                             CST use --tzone 8 (i.e. add 8 hours to UTC)
%   --mode <name> ......... : select output/import mode : 
%                              'ovldb'  : make ovl matlab db files (def.)
%                              'ovlasc' : write ovl ascii files
%   --output <path> ....... : output folder
%
% Bino Maiheu, (c) VITO 2014 
% Contact: bino.maiheu@vito.be

function opaq_import_fnl( varargin )

% -- Define command line options...
argopts = [ ...
    struct( 'name', 'domain', 'default', 'china', 'cast', @(x)(x)), ...
    struct( 'name', 'repo',   'default', 'fnl', 'cast', @(x)(x)), ...
    struct( 'name', 'tzone',  'default', 0, 'cast', @(x)(str2double(x))), ...    
    struct( 'name', 'output', 'default', '.', 'cast', @(x)(x) ), ...
    struct( 'name', 'mode',   'default', 'ovldb', 'cast', @(x)(x) ), ...
    struct( 'name', 'help',   'default', false, 'cast', NaN ) ]; % just a switch

% -- Parse and some initial checks!
[ opts, args ] = getopt_cast( argopts, '--', varargin );
if opts.help, print_usage; return; end;
if length(args) < 3, error( 'Error in arguments, try --help.' ); end;

% -- Parse the coordinates...
coords     = parse_coords( args{1} );
n_coords   = size(coords,1);

for i=1:n_coords
   fprintf( 'Requested lon=%.2f, lat=%.2f\n', coords(i,1), coords(i,2) ); 
end

% -- parse the start and end date
start_date = datenum(args{2});
end_date   = datenum(args{3});

fprintf( 'Importing from %s to %s...\n', datestr(start_date, 'yyyy-mm-dd' ), ...
    datestr(end_date, 'yyyy-mm-dd' ) );

xx_date_all  = [];
xx_meteo_all = [];

for base_date=start_date:datenum(0,0,1):end_date
    
    % -- build filename
    dv = datevec(base_date);
    ncfile = fullfile( opts.repo, sprintf( '%04d', dv(1) ), ...
        sprintf( 'fnl.%s.%s.nc', lower(opts.domain), datestr(base_date,'yyyymmdd') ) );
            
    % -- extract the OPAQ meteo from the file
    try
        fprintf( 'Processing %s...\n', ncfile );
        [ xx_date, xx_meteo ] = opaq_extract_meteo( ncfile, coords );
    catch MEx
        fprintf( '+++ error: skipping %s...\n', ncfile );
        continue;
    end
    
    if numel(xx_date) ~= 4
        fprintf( '+++ error: FNL netcdf file is not complete, missing times' );
        continue;
    end
    
    xx_date_all  = cat(1,xx_date_all,xx_date(:));
    xx_meteo_all = cat(3,xx_meteo_all,xx_meteo);
    
end

% Apply timeshift if needed...
tzone_str = 'UTC';
if opts.tzone ~= 0,
    if opts.tzone < 0, 
        tzone_str = sprintf( 'UTC -%02d:00', abs(opts.tzone) );
    else
        tzone_str = sprintf( 'UTC +%02d:00', abs(opts.tzone) );
    end,
    
    fprintf( 'Converting timestamps to %s\n', tzone_str );
    
    xx_date_all = xx_date_all + datenum(0,0,0,opts.tzone,0,0);
end


% -- Export to ovl db file
switch lower(opts.mode)
    case 'ovldb'
        for k=1:n_coords
            fprintf( 'Exporting pars for lon=%.2f, lat=%.2f\n.', coords(k,1), coords(k,2) );
            filename = fullfile( opts.output, ...
                sprintf( 'FNL_%d_%d.mat', coords(k,1), coords(k,2) ) );
   
            opaq_write_ovl_meteodb( filename, xx_date_all, squeeze(xx_meteo_all(k,:,:))', ...
                'timeZone', tzone_str );
        end
        
    case 'ovlasc'
        for k=1:n_coords
            base_name = fullfile( opts.output, ...
                sprintf( 'FNL_%d_%d', coords(k,1), coords(k,2) ) );
            
            opaq_write_ovl_ascii_meteo( base_name, xx_date_all, squeeze(xx_meteo_all(k,:,:))' );
        end
        
    otherwise
        error( 'Unknown mode : %s', opts.mode )
end

% -- Say goodbye
fprintf( 'All done, have a nice day :)\n' );



function print_usage
fprintf( 'Usage:\n');
fprintf( '   opaq_import_fnl [options] LON1,LAT1;LON2,LAT2 start_date end_date\n');
fprintf( '\n');
fprintf( ' Available options:\n');
fprintf( '   --help ................ : this message\n');
fprintf( '   --domain <name> ....... : name for the domain (def. china)\n');
fprintf( '   --repo <path> ......... : path to FNL data (excl year), (def. fnl)\n');
fprintf( '   --tzone <shift> ....... : conversion from UTC to local time zone\n');
fprintf( '                             default 0 : no conversion. for conversion to\n');
fprintf( '                             CST use --tzone 8 (i.e. add 8 hours to UTC)\n');
fprintf( '   --mode <name> ......... : select output/import mode : \n');
fprintf( '                              - ovldb  : make ovl matlab db files (def.)\n');
fprintf( '                              - ovlasc : write ovl ascii files\n');
fprintf( '   --output <path> ....... : output folder\n');
fprintf( '\n');
fprintf( ' Bino Maiheu, (c) VITO 2014 \n');
fprintf( ' Contact: bino.maiheu@vito.be\n');