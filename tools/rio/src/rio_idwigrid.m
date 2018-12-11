%RIO_IDWIGRID
% This routine performs IDW interpolation for the entire grid. No
% interpolation error is derived, value set to 0. By default the power of 4
% is used in the IDW interpolation, the x/y coordinates are taken from the
% st_info 2nd and third columns, the interpolation grid is read from the
% cnf structure ( cnf.grid_info ).
%  
%  xx_grid = rio_idwigrid( cnf, st_info, xx_data )
%
% This routine encodes some IRCEL modifications to the simple IDW routine,
% taking into account urban, industrial and traffic sites : if there are
% industrial/urban/traffic sites within 5 km, we only interpolate from
% these type of stations, if not; we remove them from the inputset
%
% See also rio_init, rio_idwgrid, rio_krigegrid
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function xx_grid = rio_idwigrid( cnf, st_info, xx_data)

%--------------------------------
%-- Grid interpolation based on 
%-- inverse Distance Weighting
%-- IRCEL modified version
%--------------------------------

%-- Power in IDW
pow = 4;

%-- Split stations in "industrial" (type >= 3) and "rural" (type 1 & 2)
ind_i = find(st_info(:,5) >= 3);
rur_i = find(st_info(:,5) <  3);

st_x = st_info(:,2)/1000.;
st_y = st_info(:,3)/1000.;

%-- Preallocate the grid
xx_grid = zeros( cnf.grid_n, 3 );

for i=1:cnf.grid_n
    %-- Grid info
    gr_id = grid_info(i,1);
    gr_x  = grid_info(i,2)/1000;
    gr_y  = grid_info(i,3)/1000;

    %-- Calc rel. dist.
    rs_x = (st_x - gr_x).^2;
    rs_y = (st_y - gr_y).^2;
    
    r = (rs_x + rs_y).^(0.5);
    
    %-- Check for urban, traffic or industrial sites
    %-- in direct vicinity (<5km)
    %-- If so, only take this sites. 
    %-- If not, remove those sites from the inputset.
    local_ind_i = intersect(find(r < 5), ind_i);
    if isempty(local_ind_i)
        %-- NO industrial sites
        xx_samps_tmp = xx_data(rur_i, :);
        r_tmp = r(rur_i);
    else
        %-- Iindustrial sites
        xx_samps_tmp = xx_data(local_ind_i, :);
        if isempty(xx_samps_tmp)
            xx_samps_tmp = cnf.missing_value;
        end
        r_tmp = r(local_ind_i);
    end
        
    %-- Weight samping values
    tmp_grid_val = xx_samps_tmp(:,2)' * r_tmp.^(- pow);
    norm = sum(r_tmp.^(-pow));
    
    xx_grid(i,1) = gr_id;
    xx_grid(i,2) = tmp_grid_val / norm;
    
    %-- Interpolation error is NOT defined!!!
    xx_grid(i,3) = 0;


end
