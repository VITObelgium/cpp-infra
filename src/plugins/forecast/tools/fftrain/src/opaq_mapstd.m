%OPAQ_MAPSTD My own simple mapping algorithm to provide input/output scaling
%
%  input_norm = opaq_mapstd( 'apply', input, PS );
%  output     = opaq_mapstd( 'reverse', output_norm, PS );
%
%  Where PS contains
%   PS.xmean  --> means of input parameters
%   PS.xstd   --> std of input parameters
%   PS.ymean  --> normally 0
%   PS.ystd   --> normally 1  (we map to std of 1 and mean of 0)
%
% Bino Maiheu, (c) 2014

function y = opaq_mapstd( mode, x, PS )

switch lower(mode)
    case 'apply'        
        y = (x-PS.xmean).*(PS.ystd./PS.xstd) + PS.ymean;
    case 'reverse'
        y = ( x-PS.ymean)./(PS.ystd./PS.xstd) + PS.xmean;
    otherwise
        error( 'OPAQ', 'mode is not implementd...');
end
