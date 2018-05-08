% -------------------------------------------------------------------------
% OVL (c) VITO / IRCEL
%
% Developed by Jef Hooybergs, Stijn Janssen, Peter Stouthuysen, Bino Maiheu
% Rewritten by Bino Maiheu (c) VITO 2010
% -------------------------------------------------------------------------
%
% Compute mean taking into account missing values & returning a success or
% not

function [ is_ok, value ] = ovl_mean( array, missing, is_ok )

index = find( array ~= missing & ~isnan( array ) );
nr    = length(index); % nr of available datapoints
if ( isempty(array) || nr < fix( length( array ) / 2 ) )
    % Apparently only compute a succesful mean if we have more than half 
    % of the data array... WHY half ????????
    is_ok = false;
    value = missing;
else
    value = mean( array(index) );
end
