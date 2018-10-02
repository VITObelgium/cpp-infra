%RIO_EXCEEDLIMITS
% Retrieves the exceedance limits for the current aggregation time and
% pollutant combination. 
%  
%  [ limit, exceed_max ] = rio_exceedlimits( agg_time, pol )
%
% In the case of 1h aggregation time, for so2 and pm10 we also return 
% the 24h limit values, by making the limit and excmax an array with 2
% values...
%
% Note that this routine is in fact deployment specific, but only the 
% EU exceedance limits are implemented in this routine.
%
% See also rio_exceedances
% 
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ limit, excmax ] = rio_exceedlimits( agg_timestr, pol )


if strcmp( agg_timestr, '1h' )
    
    % return limit values when agg_time is 1h
    switch( pol )
        case { 'pm10' }            
            % no limit on 1h values, but return limit on 24h values for
            % PM10
            limit         = [ -1, 50. ]; 
            excmax        = [ -1, 35  ];
            
        case { 'so2' }
            limit         = [ 350, 125 ];
            excmax        = [ 24,    3 ];

        case { 'no2' }
            limit         = 200.;
            excmax        = 18;
                        
        case { 'o3s' }
            limit        = [ -1, 120. ];  % last value is for m8 !
            excmax       = [ -1,  25  ];
            
        case { 'o3', 'pm25', 'bc', 'no' }
            limit         = -1;
            excmax        = -1;
            
        otherwise
            error( 'rio_exceedlimits:: unknown pollutant %s\n', cnf.pol );
    end
    
elseif strcmp( agg_timestr, 'da' )
    % return limit values when agg_time is da
    switch( pol )
        case { 'pm10' }
            limit         = 50.;
            excmax        = 35;
            
        case { 'so2' }
            limit         = 125.;
            excmax        = 3;
            
        case { 'o3', 'o3s', 'no2', 'pm25', 'bc', 'no' }
            limit         = -1;
            excmax        = -1;
            
        otherwise
            error( 'rio_exceedlimits:: unknown pollutant %s\n', pol );
    end
elseif strcmp( agg_timestr, 'm8' )
    % return limit values when agg_time is m8
    switch( pol )
        case { 'o3s' }
            limit         = 120.;
            excmax        = 25;
            
        case { 'pm10', 'no2', 'so2', 'o3', 'pm25', 'bc', 'no' }
            limit         = -1;
            excmax        = -1;
            
        otherwise
            error( 'rio_exceedlimits:: unknown pollutant %s\n', pol );
    end
    
else
    % Aggregation times m1 and m8... hmm have to have a closer look to this
    % since we probably need some generalized extreme value distribution
    % knowledge on this... donno...
    warning( 'rio_exceedlimits:: no implementation yet %s exceedances\n', agg_timestr );
    limit = -1;
    excmax = -1;
end


end
