% OPAQ_FCERR compute the forecast error base upon the hindcast
%
% fc_err = opaq_fcerr( fc_db, mode, param )
%
%
% The following modes are implemented
%
% - mode 0 : no correction is applied, the function returns 0
%
% - mode 1 : the fc error is simply the average (nanmean) of the 
%            hindcast errors
%
% - mode 2 : the hindcast errors are weighted exponentially, given larger
%            weight to the more recent errors. The shape of the exponential
%            weight is determined by the parameter 'param' (> 0). For
%            smaller values this parameter will give more wieght ot the
%            recent errors, for param -> inf this will distribute the
%            weight more equally across the hind cast. The limit of param
%            -> infinity is mode 1, the simple average... THe table below gives
%            some indication of the weight of the first and second most recent
%            errors :
%
%            param | weights...
%            ------|--------------------------
%              0   | 1.00 0.00 0.00 0.00 0.00
%              1   | 0.50 0.25 0.13 0.06 0.03
%              2   | 0.33 0.22 0.15 0.10 0.07
%              3   | 0.25 0.18 0.14 0.11 0.08
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function fc_err = opaq_fcerr( hc_db, mode, param )

switch mode
    case 0,
        % no forecast error, set it to 0 !
        fc_err = zeros( size( hc_db, 1 ), 1 );
        
    case 1,
        % simple mean 
        fc_err = nanmean( hc_db, 2 );
        fc_err( isnan(fc_err) ) = 0.;
        
    case 2,
        % exponential weighted value, based upon the formula
        %
        % \sum_{i=0}^{N-1} r^n = \frac{1-r^N}{1-r}
        %
        % this includes a fix w.r.t. the previous RTC correction scheme
        % in ovl as this had a small bias in the error. The sum of the 
        % weights is now 0. !
        
        % compute the weights
        N = size(hc_db,2); % number of hc errors
        w = wexp( param, N );
        
        % compute the weights
        hc_db( isnan(hc_db) ) = 0.;
        fc_err = hc_db * w'; 
        
        % TODO if one ore more errors in the hindcast are missing, then
        %      the exponential weights don't add up to 1 anymore, need to
        %      correct this !!
        
    otherwise
        error( 'opaq_fcerr: unknown mode : %d', mode );
end

% function to generate the exponential weights
function w = wexp( p, N )

lambda = p ./ ( 1. + p );
w = (1-lambda)*lambda.^[0:N-1]./(1-lambda.^N);
