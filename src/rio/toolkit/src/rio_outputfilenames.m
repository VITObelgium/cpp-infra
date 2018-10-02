%RIO_OUTPUTFILENAMES
% Constructs the output filenames for the requested date
% and returns them. If date is 0 (i.e. no date given), appropriate
% filenames are returned...
%
% [ grid_file, err_file, st_file ] = rio_outputfilenames( cnf, date )
%
% This is not depending on the deployment yet, can change this in the
% future...
%
% History :
%  - 2010.08.04 (BM) : added variable argument at the end so the user can put
%                      in an optional output base location
%
% See also rio_init, rio_output
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ grid_file, err_file, st_file ] = rio_outputfilenames( cnf, date, varargin )

outpath = cnf.outputPath;
if nargin > 2
    outpath = varargin{1};
end

if date == 0
    out_name = fullfile( outpath, sprintf( '%s_%s_%s-%s', ...
        cnf.pol, cnf.ipol_mode, cnf.grid_type, cnf.at_lb ) );
else
    % we have a date for the interpolation
    out_name = fullfile( outpath, sprintf( '%s_%s_%s_%s-%s', ...
        cnf.pol, cnf.ipol_mode, cnf.grid_type, datestr( date, 29 ), cnf.at_lb ) );
end
grid_file = sprintf( '%s.txt', out_name );
err_file  = sprintf( '%s-err.txt', out_name );
st_file   = sprintf( '%s-stat_val.txt', out_name );

end