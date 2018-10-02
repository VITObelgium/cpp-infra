%RIO_MAP 
% Easy RIO visualisation routine, knows the RIO grids and places
% shapefile with the belgian provinces on top of the picture. 
%
% rio_map( X )
% rio_map( X, 'param', value, ... )
% 
% Currently, it automatically selects the 4x4 grid and only supports
% this...
%
% Available parameter are : 
% 'units', 'value' ..... : sets the units to mention on the plot 
% 'title', 'value' ..... : sets the title for the plot 
% 'crange', [ x1 x2 ] .. : sets the colour range (for non fancy plot) 
% 'grid', struct ....... : gives the grid definition, make it with
%                          rio_griddef
% 'fancy', true/false .. : makes a fancy plot with cbarf and contourf
%                          instead of the normal one. Need
%                          MatlabToolkit/Graphs loaded
% 'zlevs', [...] ....... : array of z levels for fancy plot
% 'shape', 'shapefile' . : overlay a shapefile, need m_map loaded 
%
%
% See also rio_init rio_griddef
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_map( value, varargin )


p = inputParser;
p.CaseSensitive = true;
p.addParamValue( 'units', '', @isstr );
p.addParamValue( 'title', '', @isstr );
p.addParamValue( 'crange', [], @isvector );
p.addParamValue( 'grid', [], @isstruct );
p.addParamValue( 'shape', [], @isstr );
p.addParamValue( 'fancy', false, @islogical );
p.addParamValue( 'zlevs', [], @isvector );
p.addParamValue( 'st_info', [], @isstruct );
p.parse( varargin{:} );

units = p.Results.units;
tit   = p.Results.title;

if ~isempty( p.Results.grid )
    xx  = p.Results.grid.grid_info(:,2);
    yy  = p.Results.grid.grid_info(:,3);
    res = p.Results.grid.grid_res;
else
    [ xx, yy, res ] = rio_griddef;
end
% build raster
edge   = 4*res;
x      = min(xx)-edge:res:max(xx)+edge;
y      = max(yy)+edge:-res:min(yy)-edge;
[X,Y]  = meshgrid(x,y);
Z      = NaN( length(y), length(x) );
colZ   = zeros(size(Z));

for i=1:size(xx,1)      
    Z( ( X == xx(i) ) & ( Y == yy(i) ) ) = value(i);
end

% add some correction, georef does seem to be a bit off
% but only need this approximately anyway...
if ~p.Results.fancy
    pcolor( X-res/2, Y+res/2., Z );
    shading flat;
    if ~isempty( p.Results.crange )
        caxis( p.Results.crange );
    end        
    
else
    % define colormap for plot, thanks jef
    map(1,1:3)  = [0 0 255]./255;
    map(2,1:3)  = [0 126 253]./255;
    map(3,1:3)  = [0 192 0]./255;
    map(4,1:3)  = [0 255 0]./255;
    map(5,1:3)  = [204 255 51]./255;
    map(6,1:3)  = [248 231 72]./255;
    map(7,1:3)  = [255 128 0]./255;
    map(8,1:3)  = [255 0 0]./255;
    map(9,1:3)  = [192 0 0]./255;
    map(10,1:3) = [128 0 0]./255;
       
    if ~isfield( p.Results, 'zlevs' ) || isempty( p.Results.zlevs )
        p.Results.zlevs = [ 0 10 20 30 40 50 60 70 80 90 100 ];
    end
    for i=1:numel(Z),
        colZ(i) = map_code( p.Results.zlevs, Z(i) );
    end;

    pcolor( X-res/2, Y+res/2., colZ ); 
    shading flat;
    box on;
    hold on;
    
    if isfield( p.Results, 'st_info' ) && isfield( p.Results.st_info, 'val' ) 
        for i=1:numel(p.Results.st_info.val)
            st_col(i) = map_code( p.Results.zlevs, p.Results.st_info.val(i) );
        end
        
        st_cols = zeros( numel(st_col), 3 );
        for i=1:numel(st_col)
            if ~isnan(st_col(i))
                st_cols(i,:) = map(st_col(i),:);
            else
                st_cols(i,:) = [ 1 1 1 ]; % white for NaN;
            end
        end
        
        
        scatter( p.Results.st_info.x,  p.Results.st_info.y, 50, st_cols, 'filled', 'MarkerEdgeColor', [ 0 0 0 ]  );
        hold off;
    end
    
    
    colormap(map);    
    caxis( [ 1 11 ] );
    cb = colorbar( 'YTick', [1:11], 'YTickLabel', cellfun( @num2str, num2cell( p.Results.zlevs ), 'UniformOutput', false ) );
    
end


hold on;
if ~isempty( p.Results.shape )    
    p = m_shaperead( p.Results.shape );
    for k=1:length( p.ncst )
        plot( p.ncst{k}(:,1), p.ncst{k}(:,2), 'k' );
    end
end
xlabel( 'X Coordinate' );
ylabel( 'Y Coordinate' );

% Play with fonts
title( tit );

hold off;


function y = map_code( scale, x )

n = length(scale);
for i=1:n-1
   if ( ( x >= scale(i) ) && ( x < scale(i+1) ) ) 
       y = i; 
       return;
   end
end
if (x >= scale(n) ), y = n; return; end;
  
% points with error code
if ( isnan(x) || x < 0 ), y=NaN; return; end;
