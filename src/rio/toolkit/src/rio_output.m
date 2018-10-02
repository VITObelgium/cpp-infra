%RIO_OUTPUT
%  This routine outputs the interpolation grids in the selected format,
%  specified by the fmt string. The grid is simply the output of the
%  rio_interpolate routine. %
%   cnf = rio_output( cnf, vals, grid )
%   cnf = rio_output( cnf, vals, grid, fmt )
%   cnf = rio_output( cnf, vals, grid, fmt, date )
%   cnf = rio_output( cnf, vals, grid, fmt, date, outpath )
%
%  The format string can be : 
%   fmt : 'RIO' : standard RIO ascii files
%
%  A number of configuration structure parameters are relevant here : 
%   - cnf.outputPath : location to store the output files, the files are written
%                   directly into this folder, without going into any subfolders.
%   - cnf.outputXY : store the XY locations for each interpolation value in
%                   the grid output files
%   - cnf.output_init : is false when the routine is called the first time
%                   with a new cnf structure. then the persistent variables
%                   are re-initialised and the flag is set true...
%
%  Notes
%   - Output header fields for 1h interpolations, so far I'm using the 
%     loop variable to determine the hour of interpolation, also the
%     internal handling of the 1h data assumes you start from the 0th hour
%     on that day, so doesn't fully support e.g. single interpolations of
%     say the values at 16:00 when using dates to select the data from a
%     history database...
%
% Changelog :
%  - 2010.08.04 BM : added extra optional output path location
%
% See also rio_init, rio_interpolate, rio_outputfilenames, 
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function cnf = rio_output( cnf, vals, grid, varargin )
% declare the following variables as persistent in the output routine
% this makes the construction of the output grids according to
% specifications and the addition of new 1h values easier...
persistent export_grid 
persistent export_err 
persistent export_stat  
persistent export_num   % number of interpolations in current file
persistent export_date  % current date which is being exported, only yyyy,mm,dd

if ~cnf.output_init
    % first time this routine is called with the new config, clear persistent
    % variables
    export_grid  = [];
    export_err   = [];
    export_stat  = [];
    export_num   = [];
    export_date  = [];
    cnf.output_init = true;
end

fmt  = 'RIO';
date = 0;
outpath = cnf.outputPath;

% need to find something more elegant for this rubbish : 
if nargin > 3
    fmt = varargin{1};
    if nargin > 4
        date = varargin{2};
        if nargin > 5
            outpath = varargin{3};
        end
    end
end

% Construct output filename...
[ grid_file, err_file, st_file ] = rio_outputfilenames( cnf, date, outpath );

% construct output grid
% Append (1h) or start new file...
if ( ( cnf.agg_time == 4 ) && isequal( floor( date ), export_date ) )
    %-- We have to append to the existing 1h file...
    fprintf( 'rio_output:: appending to current output %s\n', grid_file );
    export_grid = [export_grid grid(:,2)];
    export_err  = [export_err  grid(:,3)];
    export_stat = [export_stat vals(:,2) ];
    export_num  = export_num + 1;
else
    fprintf( 'rio_output:: writing new output %s\n', grid_file );
    if cnf.outputXY
        export_grid = [grid(:,1) cnf.grid_info(:,2:3) grid(:,2)];
        export_err  = [grid(:,1) cnf.grid_info(:,2:3) grid(:,3)];
        export_stat = [cnf.st_info(:,2:3) vals(:,2) ];
    else
        export_grid = [grid(:,1) grid(:,2)];
        export_err  = [grid(:,1) grid(:,3)];
        export_stat = vals(:,2);
    end
    export_num    = 1;
    export_date   = floor( date );
end

%-- Write out new file for this date, for the hourly we overwrite the
%   already existing file, but hey... that's okay :), first set the
%   format for the outputfiles

[ head_grid, head_err, fmt_grid, head_stat, fmt_stat ] = ...
    get_fmtstr( export_num, cnf.outputXY, cnf.pol, cnf.at_lb );

%-- Stations ID's
conv_id  = char( cnf.st_id );

%-- Handle different output formats...
switch ( fmt )
    
    case 'RIO'        
        %-- Write the grid...
        fid = fopen(grid_file, 'wt');
        fprintf(fid, head_grid );
        fprintf(fid, fmt_grid, export_grid');
        fclose(fid);
        
        %-- Export interpolation error...
        fid = fopen(err_file, 'wt');
        fprintf(fid, head_err );
        fprintf(fid, fmt_grid, export_err');
        fclose( fid );
        
        %-- Export station info...
        fid = fopen(st_file, 'wt');
        fprintf(fid, head_stat );
        for k=1:cnf.nr_st
            fprintf(fid, fmt_stat, conv_id(k,:), export_stat(k,:) );
        end
        fclose(fid);               
        
    otherwise
        error( 'rio_output:: unknown output format %s !', fmt );
end

end


% Little helper function to get the header and the format for the output
% of the grid...
function [ head_grid, head_err, fmt_grid, head_stat, fmt_stat ] = ... 
    get_fmtstr( n, xy, pol, at_lb )        
% Sets the output format for the datafiles for both the grid output
% and the station outputfiles...

if xy
    % Output format with coordinates
    head_grid = 'ID;X;Y';
    head_err  = 'ID;X;Y';
    head_stat = 'STATCODE;X;Y';
        
    % fmt_grid = '%.2f;%.1f;%.1f';
    fmt_grid  = '%d;%d;%d';    
        
    %fmt_stat = '%s;%.1f;%.1f';
    fmt_stat = '%s;%d;%d';

else
    % Output format without coordinates
    head_grid = 'ID';
    head_err  = 'ID';
    head_stat = 'STATCODE';    
     
    %fmt_grid = '%.2f';
    fmt_grid = '%d';
    fmt_stat = '%s';
    
end

% n = number of columns in the data array... normally 1, unless
% agg_time = 4, then the number of hours already processed of the 
% day...
for k=1:n,    
    if strcmpi( at_lb, '1h' ), 
        lb = sprintf( '%dh', k-1 );
    else
        lb = at_lb;
    end
    
    head_grid = [ head_grid, sprintf( ';%s_%s',     pol, lb ) ];
    head_err  = [ head_err,  sprintf( ';%s_err_%s', pol, lb ) ];
    head_stat = [ head_stat, sprintf( ';%s_%s',     pol, lb ) ];    
    
    %fmt_grid = [ fmt_grid, ';%.5f' ];
    fmt_grid = [ fmt_grid, ';%.4f' ];
    fmt_stat = [ fmt_stat, ';%.4f' ];
end;

head_grid = [ head_grid, '\n' ];
head_err  = [ head_err,  '\n' ];
head_stat = [ head_stat, '\n' ];

fmt_grid = [ fmt_grid, '\n' ];
fmt_stat = [ fmt_stat, '\n' ];

end