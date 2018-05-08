%OPAQ_HCERR compute hindcast error database
%
% hc_err = opaq_hcerr( xx_date, xx_value, fc_value, fc_hor, hc_range )
%
% Somewhat simpler version of the hindcast error calculation, but way slower
% best stick to the ovl_hcerr version...
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function hc_err = opaq_hcerr( xx_date, xx_value, fc_value, fc_hor, hc_range )

% the size of the error data base
hc_err = nan( size(xx_date,1), hc_range );

% compute the errors, note these are fc_hor
xx_error = fc_value - xx_value;

% now construct the error db for the operational forecast
for i=1:length(xx_date)
    for j=1:hc_range
        dayN = xx_date(i)-fc_hor-j;
        idx  = find( xx_date == dayN );
        
        if ~isempty( idx ), hc_err(i,j) = xx_error(idx); end;
    end
end
