%RIO_TRENDFCN
% This function returns a rio trendfunction doublet ( value + error ) 
% of the specified order. Returned are matlab function handles for the 
% trends. 
%
% [ fcn, fcn_err, np, np_err, ord_err ] = rio_trendfcn( order )
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ fcn, fcn_err, np, np_err, ord_err ] = rio_trendfcn( order )

switch order
    case 1,
        fcn     = @(p,x) p(1)*x    + p(2);
        np      = 2;
        fcn_err = @(p,x) p(1)*x.^2 + p(2)*x  + p(3);        
        np_err  = 3;
        ord_err = 2;
    case 2,
        fcn     = @(p,x) p(1)*x.^2 + p(2)*x    + p(3);
        np      = 3;
        fcn_err = @(p,x) p(1)*x.^4 + p(2)*x.^3 + p(3)*x.^2 + p(4)*x + p(5);
        np_err  = 5;
        ord_err = 4;
    otherwise
        error( 'rio_trendfcn:: polygon order too high, should be <= 2 !!' );
end
        
