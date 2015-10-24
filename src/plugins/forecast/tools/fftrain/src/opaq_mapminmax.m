%OPAQ_MAPMINMAX My own simple mapping algorithm to provide input/output scaling
%
%  input_norm = opaq_mapminmax( 'apply', input, PS );
%  output     = opaq_mapminmax( 'reverse', output_norm, PS );
%
%  Where PS contains
%   PS.xmin  --> min of input
%   PS.xmax  --> max of input
%   PS.ymin  --> normally -1
%   PS.ymax  --> normally 1
%
% Bino Maiheu, (c) 2014

function y = opaq_mapminmax( mode, x, PS )

switch lower(mode)
    case 'apply'        
        y = (PS.ymax-PS.ymin).*(x-PS.xmin)./(PS.xmax-PS.xmin) + PS.ymin;        
    case 'reverse'        
        % here y is in fact x and vice versa
        y = (PS.xmax-PS.xmin).*(x-PS.ymin)./(PS.ymax-PS.ymin) + PS.xmin;        
        
    otherwise
        error( 'OPAQ', 'mode is not implementd...');
end