%RIO_GRIDDEF
%
% Generates ( or updates ) a configuration structure with the relevant
% grid parameters filled in from the input arrays. Can be used in 
% conjunction with rio_display for easy displaying maps of the
% concentrations.
% 
% Example:
%   cnf = rio_griddef( X, Y, 4000 )
%
% Where X and Y are 1D arrays which hold the x,y coordinates of the
% gridcells. Alternatively, one can also enter the gridtype, in which
% case the griddefinition is loaded autmatically from the stored
% rio_grids.mat file. If you specify 3 output arguments it will return
% the X, Y, and resolution of the selected grid.
%
% See also rio_init rio_display rio_showgrid
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

%function cnf = rio_griddef( X, Y, resol, varargin )
function varargout = rio_griddef( varargin )

if nargin == 0
    g = load( 'rio_grids' );
    fprintf( 'Loading 4x4 griddefinition...\n' );            
    X = g.gr_4x4.X;
    Y = g.gr_4x4.Y;
    resol = g.gr_4x4.res;
    clear g;
elseif nargin == 1 && ~isempty(varargin{1})
    g = load( 'rio_grids' );
    switch( varargin{1})
        case {'4x4', 'RIO'}, fprintf( 'Loading 4x4 griddefinition...\n' );            
            X     = g.gr_4x4.X;
            Y     = g.gr_4x4.Y;
            resol = g.gr_4x4.res;
        case '1x1', fprintf( 'Loading 1x1 griddefinition...\n' );            
            X     = g.gr_1x1.X;
            Y     = g.gr_1x1.Y;
            resol = g.gr_1x1.res;   
        case {'3x3', 'belEUROS'}, fprintf( 'Loading belEUROS griddefinition...\n' );            
            X     = g.gr_belEUROS.X;
            Y     = g.gr_belEUROS.Y;
            resol = g.gr_belEUROS.res;
        otherwise
            error( 'MATLAB:rio_griddef', 'Invalid grid specifier...' );            
    end
    clear g; 
elseif nargin == 3 
    X     = varargin{1};
    Y     = varargin{2};
    resol = varargin{3};
else
    error( 'MATLAB:rio_griddef', 'Invalid number of arguments...')
end

% generate basic configuration structure
if size(X) ~= size(Y)
    error( 'MATLAB:rio_griddef', 'Invalid gridsizes between X and Y...' );
    end

if nargout == 1
    gr_n = size(X,1);
    cnf  = struct( 'grid_info', zeros( gr_n, 3 ), ...
                   'grid_res', 0, ...
                   'grid_n', 0 );    

    cnf.grid_info(:,1) = 1:gr_n;
    cnf.grid_info(:,2) = X;
    cnf.grid_info(:,3) = Y;
    cnf.grid_res       = resol;
    cnf.grid_n         = gr_n;
    varargout{1} = cnf;
elseif nargout == 3
    varargout{1} = X;
    varargout{2} = Y;
    varargout{3} = resol;
else
    error( 'MATLAB:rio_griddef', 'Invalid number of output arguments...' );
end

