%RIO_IDWGRID
% This routine performs IDW interpolation for the entire grid. No
% interpolation error is derived, value set to 0. By default the power of 4
% is used in the IDW interpolation, the x/y coordinates are taken from the
% st_info 2nd and third columns, the interpolation grid is read from the
% cnf structure ( cnf.grid_info ). 
%  
%  xx_grid = rio_idwgrid( cnf, st_info, xx_data )
%
% See also rio_krigegrid, rio_idwigrid
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function xx_grid = rio_idwgrid( cnf, st_info, xx_data )

%--------------------------------
%-- Grid interpolation based on 
%-- inverse Distance Weighting
%--------------------------------

%-- Power in IDW
pow = 4;

st_x = st_info(:,2)/1000.;
st_y = st_info(:,3)/1000.;

%-- Preallocate the grid
xx_grid = zeros( cnf.grid_n, 3 );

for i=1:cnf.grid_n
    %-- Grid info
    gr_id = cnf.grid_info(i,1);
    gr_x  = cnf.grid_info(i,2)/1000;
    gr_y  = cnf.grid_info(i,3)/1000;

    %-- Calc rel. dist.
    rs_x = (st_x - gr_x).^2;
    rs_y = (st_y - gr_y).^2;
    
    r = (rs_x + rs_y).^(0.5);
    
    %-- Weight samping values
    tmp_grid_val = xx_data(:,2)' * r.^(- pow);
    norm = sum(r.^(-pow));
    
    xx_grid(i,1) = gr_id;
    xx_grid(i,2) = tmp_grid_val / norm;
    
    %-- Interpolation error is NOT defined!!!
    xx_grid(i,3) = 0;
end



end
