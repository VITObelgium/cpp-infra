%RIO_TYPEPLOT
% Small helper function to create a plot of the averaged or std values, but
% differentiated by station type.
%
% rio_typeplot( x, y, type )
% rio_typeplot( x, y, type, plot_legend )
% rio_typeplot( x, y, type, plot_legend, col_x, col_f, type_lb, st_types )
% 
% Default values for colour, marker types and station types are : 
%   col_x    = { 'ob'    '^g'       'sm'  'dr'  'vk'    };
%   col_f    = { 'b'     'g'        'm'   'r'   'k'     };
%   type_lb  = { 'rural' 'urb back' 'urban' 'indust' 'traff' };
%   st_types = [ 1 2 3 4 5 ]; % possible station types
%
% When a type is encoutered which doesn't exist in the station types array,
% a 'k.' unknown is plotted !
%
% See also rio_plottrend, rio_gettrend
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_typeplot( x, y, type, varargin )

% -- Colouring & makers for station types
plot_legend = false;
col_x    = { 'ob'    '^g'       'sm'  'dr'  'vk'    };
col_f    = { 'b'     'g'        'm'   'r'   'k'     };

type_lb  = { 'rural' 'urb back' 'urban' 'indust' 'traff' };
st_types = [ 1 2 3 4 5 ]; % possible station types

% -- Non-default plotting options...
if nargin > 3
    if nargin == 4
        plot_legend = varargin{1};
    elseif nargin == 8
        plot_legend = varargin{1};
        col_x       = varargin{2};
        col_f       = varargin{3};
        type_lb     = varargin{4};
        st_types    = varargin{5};
    else
        error( 'rio_typeplot: invalid arguments.' );
    end
end

leg_disp = {};
leg_handle = [];

% -- Select all the types and plot   
for i=1:length( st_types )
    type_i = find( type == st_types(i) );
    if ~isempty(type_i)
        if i>1, hold on; end        
        leg_handle(end+1) = plot( x(type_i), y(type_i), char( col_x(i) ), 'MarkerFaceColor', char( col_f(i) ) );
        leg_disp(end+1) = type_lb(i);
    end
end

% -- Plot the unknowns
type_i = find( ~ismember( type, st_types ) );
if any( type_i )
    leg_handle(end+1) = plot( x(type_i), y(type_i), '.k', 'MarkerFaceColor', 'k' );
    leg_disp{end+1} = 'unknown';
end

if plot_legend
    legend( leg_handle, leg_disp, 'Location', 'Best', 'FontSize', 7 );
end
