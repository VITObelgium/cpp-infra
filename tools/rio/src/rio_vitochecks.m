%RIO_VITOCHECKS
% This routine performs some checks on the VITO deployment of RIO
%
%  [ tf ] = rio_vitochecks( cnf )
%
% EMPTY ROUTINE, NO IMPLEMENTATION
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function tf = rio_vitochecks( c )
       
tf = true;

% Checking the requested grid & proxy type & grid combination for existance
%if ( ~strcmp( c.proxyType, 'CorineID' ) && ...
%        ~strcmp( c.proxyType, 'CorineID_double_beta' ) && ...
%        ~strcmp( c.proxyType, 'AOD' ) )    
%    fprintf( 'rio_vitochecks:: check bla bla failed....\n' );
%    passed = false;
%    return;
%end

end