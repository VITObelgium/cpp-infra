% -------------------------------------------------------------------------
% OVL (c) VITO / IRCEL
%
% Developed by Jef Hooybergs, Stijn Janssen, Peter Stouthuysen, Bino Maiheu
% Rewritten by Bino Maiheu (c) VITO 2010
% -------------------------------------------------------------------------
%
% Compute maximum taking into account missing values & returning a success or
% not

function [ is_ok, value ] = ovl_max( array, missing, is_ok )

index = find(array ~= missing & ~isnan( array )  );
nr    = length(index); % nr of available datapoints

if ( isempty(array) || nr < fix( length( array ) / 2 ) )
    is_ok = false;
    value = missing;
else
    value = max( array( index ) );
end

