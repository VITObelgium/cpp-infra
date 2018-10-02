%RIO_AVGTREND
%  Returns the trend function for the requested station indicators, 
%  depending on the configuration, also returns the reference level
%  for the trend on the expectation values...
%
%  [fit, ref_level] = rio_avgtrend( cnf, st_indic )
%
% Changelog
%  - 17.05.2011 [BM] : merged in the changes from the new driver clc06d for PM10
%                      and the AOD threshold
%
% See also rio_init, rio_checkdeployment, rio_stdtrend
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [fit, ref_level] = rio_avgtrend( cnf, st_indic )

%-----------------------------------------
%-- Trend for TS expectation values
%--
%-- Note that also the reference level (ref_level) is
%-- returned in order to calculate the
%-- trend shift (ref_level - y).
%-----------------------------------------

switch cnf.configName
    
    
    
    case { 'v3.4', 'lts' }
        % =====================================================================
        % Configuration for older versions then RIO 3.6 (IRCEL)
        % Bino Maiheu, 2013-10-21
        % =====================================================================
        switch cnf.deployment
            
            case { 'VITO', 'IRCEL' }
                %-- Define plateau value in x!!
                %-- ATTENTION: if beta values change,
                %-- these plateau levels have to be
                %-- redetermined
                %-- X_FLAT: beyond this value a
                %-- constant plateau (FLAT_LEVEL) will be used...
                
                %-- Add the fix for low O3 in the harbour areas...
                %-- Firstly, set the reference levels and flat levels
                if (strcmp(cnf.pol_xx, 'o3') || strcmp(cnf.pol_xx, 'o3s'))
                    ref_level = 80; % 80 voor rapport RIO-2007 vs.1
                    switch cnf.gis_type
                        case 'Pop'
                            x_flat = 6;
                        case { 'CorineID', 'clc06d' }
                            x_flat = 0;
                            x_max  = 0.91;
                    end
                elseif strcmp(cnf.pol_xx, 'no2')
                    ref_level = 70; %70 voor rapport RIO-2007 vs.1
                    x_flat= 1.4;
                    
                elseif strcmp(cnf.pol_xx, 'pm10')
                    switch cnf.gis_type
                        case { 'CorineID', 'clc06d' }
                            x_flat= 1;
                            ref_level = 40; %40 voor rapport RIO-2007 vs.1
                        case 'AOD'
                            x_flat= 0.3;
                            ref_level = 40; %40 voor rapport RIO-2007 vs.1
                    end
                    
                elseif strcmp(cnf.pol_xx, 'pm25')
                    switch cnf.gis_type
                        case 'CorineID'
                            error( 'not implemented' );
                        case 'AOD'
                            x_flat = 0.3;
                            ref_level = 20; % BM 2011.01.18
                        case 'AODBETA'
                            x_flat = 0.3;
                            ref_level = 20; % BM 2011.01.18
                    end
                elseif strcmp(cnf.pol_xx, 'so2')
                    ref_level = 20; %20 voor rapport RIO-2007 vs.1
                    x_flat= 2.7;
                end
                
                %-- Now get the trend...
                if ( strcmp(cnf.gis_type, 'CorineID') || ...
                        strcmp(cnf.gis_type, 'AOD') || ...
                        strcmp(cnf.gis_type, 'AODBETA') || ...
                        strcmp(cnf.gis_type, 'clc06d' ) )
                    %-- Cast trend in parabolic form a.(x-b)^2+c
                    a = cnf.p_avg(1);
                    b = -cnf.p_avg(2) ./ (2*a);
                    c = cnf.p_avg(3) - (cnf.p_avg(2).^2)./(4*cnf.p_avg(1));
                    
                    fit = a * (st_indic - b).^2 + c;
                    
                    %-- Forcing of the plateau...
                    if (strcmp(cnf.pol_xx, 'o3') || strcmp( cnf.pol, 'o3s' ) )
                        if a < 0
                            index_plateau = find((st_indic < max(b, x_flat)) == 1);
                            flat_level    = a * (max(b, x_flat) - b)^2 + c;
                        elseif a >= 0
                            index_plateau = find((st_indic < x_flat) == 1);
                            flat_level = a * (x_flat - b)^2 + c;
                        end
                        % add the check on the max st_indic parameter for CorineID
                        % or clcl06d
                        if ( strcmp( cnf.gis_type, 'CorineID') || strcmp( cnf.gis_type, 'clc06d') )
                            index_max        = find((st_indic > x_max) == 1);
                            max_level        = a * ( x_max - b )^2 + c;
                            fit( index_max ) = max_level;
                        end
                        
                    elseif strcmp(cnf.pol_xx, 'no2') || strcmp(cnf.pol_xx, 'pm10') ...
                            || strcmp(cnf.pol_xx, 'so2') || strcmp(cnf.pol_xx, 'pm25')
                        if a < 0
                            index_plateau = find((st_indic > min(b, x_flat)) == 1);
                            flat_level = a * (min(b, x_flat) - b)^2 + c;
                        elseif a >= 0
                            index_plateau = find((st_indic > x_flat) == 1);
                            flat_level = a * (x_flat - b)^2 + c;
                        end
                    end
                    
                    fit(index_plateau) = flat_level;
                    
                elseif ( strcmp( gis_type, 'CorineID_double_beta' ) )
                    
                    % Surface trend for double beta indicator, surfaces defined in
                    % config structure
                    if strcmp( cnf.pol_xx, 'pm10')
                        fit  = cnf.plane_lin( cnf.p_avg, st_indic );
                    else
                        fit  = cnf.plane_2nd( cnf.p_avg, st_indic );
                    end
                    
                elseif (strcmp(gis_type, 'Pop'))
                    % Linear trend for population
                    fit = cnf.p_avg(1)*st_indic+p_avg(2);
                end
                
                
            case { 'VMM-NH3' }
                % trend handling for VMM-NH3 deployment...
                ref_level = 5;   % found this value in RIO-NH3 pre now , putting in 5.. is better !
                x_flat    = 1.5; % or top of the parabola
                
                a = cnf.p_avg(1);
                b = -cnf.p_avg(2) ./ (2*a);
                c = cnf.p_avg(3) - (cnf.p_avg(2).^2)./(4*cnf.p_avg(1));
                
                fit = a * (st_indic - b).^2 + c;
                
                if a < 0
                    index_plateau = find((st_indic > min(b, x_flat)) == 1);
                    flat_level = a * (min(b, x_flat) - b)^2 + c;
                elseif a >= 0
                    index_plateau = find((st_indic > x_flat) == 1);
                    flat_level = a * (x_flat - b)^2 + c;
                end
                fit(index_plateau) = flat_level;
                
            otherwise
                error( 'rio_avgtrend:: deployment not defined in this function!' );
        end
        
    otherwise
        % =====================================================================
        % Configuration for version 3.6 or other
        % Bino Maiheu, 2013-10-21
        % =====================================================================
        if ~isfield( cnf, 'avgTrend' )
            error( 'We expect the avgtrend in the XML configuration for this version...' );
        end
        
        ref_level = cnf.avgTrend.ref_level;
        
        switch( cnf.avgTrend.type )
            
            case 'poly2'
                % ----------------------------------------------
                %  2nd degree polynomial trend
                % ----------------------------------------------
                fit = rio_poly2( cnf.p_avg, st_indic, cnf.avgTrend.indic_lo, cnf.avgTrend.indic_hi );
                
            case 'poly1'
                % ----------------------------------------------
                %  1st degree polynomial trend
                % ----------------------------------------------
                fit  = rio_poly1( cnf.p_avg, st_indic, cnf.avgTrend.indic_lo, cnf.avgTrend.indic_hi );
                
            otherwise
                error( 'This trend is not yet implemented...' );
        end
        
end % end switch configName


% TODO : Check whether applying a logarithmic tranformation to reference
%        level when using the logtrans option is ok...
if cnf.Option.logtrans,
    ref_level = log( 1. + ref_level );
end

end

