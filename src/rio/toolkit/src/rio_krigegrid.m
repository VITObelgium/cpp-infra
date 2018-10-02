%RIO_KRIGEGRID
% This routine performs Kriging for the entire grid, it calls the rio_krige
% routine internally
%  
%  xx_grid = rio_krigegrid( cnf, C_inv, st_info, xx_data )
%
% The returned xx_grid contains 3 columns : the grid ID, the interpolated
% value and the uncertainty. 
%
% Notes :
%  This routine can still be coded more efficiently, making use of the
%  matlab vectorisation instead of haveing an explicit for loop !!!
%
% See also rio_init, rio_covmat, rio_krige
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function xx_grid = rio_krigegrid( cnf, C_inv, st_info, xx_data )

%-- Preallocate the grid
xx_grid = zeros( cnf.grid_n, 3 );

for i=1:cnf.grid_n
    gr_id    = cnf.grid_info(i,1);
    gr_x     = cnf.grid_info(i,2)/1000.;
    gr_y     = cnf.grid_info(i,3)/1000.;
    
    
    %-- Calculate interpolation value...
    [ krig_val, krig_err ] = rio_krige( cnf, C_inv, st_info, xx_data, gr_x, gr_y );
    
    
    xx_grid(i,1) = gr_id;
    xx_grid(i,2) = krig_val;
    xx_grid(i,3) = krig_err;
    
end

end
