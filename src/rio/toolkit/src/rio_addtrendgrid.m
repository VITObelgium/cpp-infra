%RIO_ADDTRENDGRID
%  Add the trend to the entire grid, calls rio_addtrend to actually
%  calculate the retrending...
%
%  xx_grid = rio_addtrendgrid( cnf, xx_grid )
%
% See also rio_init, rio_addtrend
% 
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function xx_grid = rio_addtrendgrid( cnf, xx_grid )

for i=1:cnf.grid_n       
    
    if strcmp( cnf.gis_type, 'CorineID_double_beta' )
        gr_indic = [ cnf.grid_info(i,4) cnf.grid_info(i,5) ];
    else
        gr_indic = cnf.grid_info(i,4);
    end
    
    %-- Check for missing values in gr_indic data...
    if gr_indic(1,:) >= 0
        
        % xx_grid(i,2) contains the kriging value
        % xx_grid(i,3) contains the kriging error
        
        [ xx, xx_err ] = rio_addtrend( cnf, xx_grid(i,2), xx_grid(i,3), gr_indic );
        
        xx_grid(i,2) = xx;
        xx_grid(i,3) = xx_err;
        
    else
        
        xx_grid(i,2) = cnf.missing_value;
        xx_grid(i,3) = 0.;
        
    end
    
end

end
