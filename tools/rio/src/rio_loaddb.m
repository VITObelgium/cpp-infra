%RIO_LOADDB
% This routine loads the historic database with measurements corresponding
% to the current configuration and attaches it to the config structure
%
% cnf = rio_loaddb( cnf )
% 
% See also rio_init, rio_createdb, rio_loadgrid, rio_loadstationinfo
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function Cnf = rio_loaddb( Cnf )


%--------------------------------
%-- Load historical events 
%-- 
%-- structure "xx_val":
%-- for max values
%-- [st_id ; xx_max1h ; xx_max8h; day avg]
%--
%-- for 1h values:
%-- [st_id ; 24 * 1h_val]
%--
%--------------------------------

%-- Define variables
xx_date_file = fullfile( Cnf.dbasePath, sprintf( '%s_days.mat', Cnf.pol_xx ) );
if Cnf.agg_time <= 3
    xx_mat_file = fullfile( Cnf.dbasePath, sprintf( '%s_data_max.mat', Cnf.pol_xx ) );
else
    xx_mat_file = fullfile( Cnf.dbasePath, sprintf( '%s_data_1h.mat', Cnf.pol_xx ) );
end

if ~exist( xx_mat_file, 'file' ) 
    Cnf.errcode = 1;
    Cnf.errmsg  = sprintf( 'file not found %s', xx_mat_file );
    return;
else
   fprintf( 'Loading historic database from %s\n', xx_mat_file ) ;
end

if ~exist( xx_date_file, 'file' ) 
    Cnf.errcode = 1;
    Cnf.errmsg  = sprintf( 'file not found %s', xx_date_file );
    return;
else
   fprintf( 'Loading matching dates for historic database from %s\n', xx_date_file ) ;
end

xx_val_struc  = load( xx_mat_file );
xx_date_struc = load( xx_date_file );

if Cnf.agg_time <= 3
    Cnf.xx_val = xx_val_struc.xx_val_max;
else
    Cnf.xx_val = xx_val_struc.xx_val_1h;
end

Cnf.xx_date = xx_date_struc.xx_date;

%-- Correct for missing values. Are replaced by NaN's
%warndlg('Warning: remove -9999','CHECK!') 
%Cnf.xx_val( Cnf.xx_val == -9999 ) = NaN;
Cnf.xx_val( Cnf.xx_val < 0 ) = NaN;

% Set the flag to let RIO know we have a historic dataset in memory
Cnf.have_db = true;

end