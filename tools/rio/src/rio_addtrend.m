%RIO_ADDTREND
%  Workhorse routine to add the trend with the uncertainty back to a single
%  kriged value.
%
%  [ xx, xx_err ] = rio_addtrend( cnf, krig_val, krig_err, gr_indic )
%
% See also rio_init, rio_detrend, rio_stdtrend, rio_avgtrend
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ xx, xx_err ] = rio_addtrend( cnf, krig_val, krig_err, gr_indic )

%--------------------------------------------------------
%-- Add trend and covert to xx value for location (x,y)
%--------------------------------------------------------

%-- Rescale factors...
%----------------------
[ std_tr, std_ref ]  = rio_stdtrend( cnf, gr_indic );  

%-- Trend shifts...
%-------------------
[ fit, ref_level ] = rio_avgtrend( cnf, gr_indic );  

%-- Calculate trend shift...
delta_tr = ref_level - fit;

%-- Trend errors...
%------------------- 
if ~strcmp( cnf.gis_type, 'CorineID_double_beta' )
   
    avg_trend_err = polyval( cnf.p_avg_err, gr_indic );
    std_trend_err = polyval( cnf.p_std_err, gr_indic );

else
    
    % Trend surfaces are stored in the config file...
    if strcmp( cnf.pol_xx, 'pm10')
        avg_trend_err = cnf.plane_2nd( cnf.p_avg_err, gr_indic );
        std_trend_err = cnf.plane_2nd( cnf.p_std_err, gr_indic );
    else
        avg_trend_err = cnf.plane_4th( cnf.p_avg_err, gr_indic );
        std_trend_err = cnf.plane_2nd( cnf.p_std_err, gr_indic );
    end
    
end

%-------------------------------------
%-- transformation of Kriging value
%-------------------------------------

%-- add trend shift in mean...  
xx_tmp = krig_val - delta_tr;

%-- rescale according to trend in std. dev.
xx = (xx_tmp - fit) ./ std_tr + fit;

%-- Correct for negative values...
%-- Put the detection limit in here ( fix RIO 2009...) --> we put it in the
%   program
xx( xx < 0 ) = 0;

%-------------------------------------
%-- transformation of Kriging error
%-------------------------------------
xx_err = sqrt( ( krig_err ./ std_tr ).^2 + ( avg_trend_err  ).^2 + ...
    ( std_trend_err .* ( krig_val - ref_level ) ./ std_ref ).^2 );


end
