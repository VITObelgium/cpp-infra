%RIO_CHECKDEPLOYMENT
% This routine checks and sets some deployment specific parameters.
% Eventually it should be removed from the rio library and the deployment
% specific parameters should be moved to configuration files !
%  
% cnf = rio_checkdeployment( cnf )
%
% History
% - 2010.04.19 [BM] : moved the o3s -> o3 translation in here for IRCEL and
%                     VITO deployments..
%
% See also rio_init, rio_avgtrend
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ Config ] = rio_checkdeployment( Config )

Config.errcode = 0;
Config.errmsg  = '';

fprintf( 'Checking %s deployment\n', Config.deployment );
switch( Config.deployment )
    case 'VITO'
        if ~rio_vitochecks( Config );
            Config.errcode = 1;
        end
        % Cnf.pol = user requested pollutant
        % Cnf.pol_xx = used in the code, normally the same, but exeption for
        % O3 summer
        if strcmp( Config.pol, 'o3s')
            Config.pol_xx = 'o3';
        end
        
    case 'IRCEL'
        % add  some routine here for each deployment...
        % Cnf.pol = user requested pollutant
        % Cnf.pol_xx = used in the code, normally the same, but exeption for
        % O3 summer
        if strcmp( Config.pol, 'o3s')
            Config.pol_xx = 'o3';
        end
        
    otherwise
        fprintf( 'rio_checkdeployment:: unknown deployment, continue anyway...\n' );        
end





end