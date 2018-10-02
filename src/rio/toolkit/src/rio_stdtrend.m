%RIO_STDTREND
% Returns the trend function for the requested station indicators,
% depending on the configuration, also returns the reference level
% for the trend on the expectation values...
%
%  [ y, ref_level] = rio_stdtrend( cnf, st_indic )
%
% 
% Adjusted the calculation of the reference level when using the full
% log transformed version of RIO.
%
% See also rio_init, rio_checkdeployment, rio_avgtrend
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ y, ref_level ] = rio_stdtrend( cnf, st_indic )

%---------------------------------------------
%-- Trend for TS standard deviation
%--
%-- Trend cast in form:
%-- y = a.beta + b
%-- or z = a.beta_small + b.beta_large + c.beta_small.beta_large + d

%--
%-- Note that the function returns the
%-- fraction of a fixed std. dev. ("rel_level") and the
%-- std. dev. given by the trend!
%---------------------------------------------


% If we want to use the log transform in the calculation, 
% we need to convert the reference level for the standard deviation
% of te concentrations to a reference level for the log(1+X) transform
% of the concentrations. 
%
% It is know that (CHECK THIS !!!): 
% VAR[f(X)] ~ (f'(E[X]))^2*VAR[X] (first order Taylor)
% VAR[f(X)] ~ (f'(E[X]))^2*VAR[X] + [f''(E[X])]^2/4*VAR^2[X] (second order Taylor)
%
% So to first order approximation : 
%   VAR[log(1+X)] ~ VAR[X]/(1+E[X])^2
%
% and therefore we need the expectaion value of the reference level (given
% by rio_avgtrend :
if cnf.Option.logtrans
    [ dummy, avg_ref ] = rio_avgtrend( cnf, st_indic );
end

switch cnf.configName
    
    case { 'v3.4', 'lts' }
        % =====================================================================
        % Configuration for older versions then RIO 3.6 (IRCEL)
        % Bino Maiheu, 2013-10-21
        % =====================================================================
        switch cnf.deployment
            
            case { 'VITO', 'IRCEL' }
                
                %-- Define reference value in x
                switch cnf.pol_xx
                    case {'o3', 'o3s'}
                        ref_level = 30;
                    case 'no2'
                        % bugfix, made the same as in RIO fortran...
                        ref_level = 15;
                    case 'pm10'
                        ref_level = 10;
                    case 'pm25'
                        ref_level = 10;
                    case 'so2'
                        ref_level = 5;
                end
                
                % See remarks above : VAR(log(1+X)) = VAR(X)/(1+X)^2
                if cnf.Option.logtrans
                    ref_level = ref_level ./ ( 1 + avg_ref );
                end
                
                
                %-- Calculate fraction...
                if ~isempty(st_indic)
                    if ~strcmp( cnf.gis_type, 'CorineID_double_beta' )
                        y = ref_level ./ (cnf.p_std(1)*st_indic + cnf.p_std(2));
                    else
                        y = ref_level ./ cnf.plane_lin( cnf.p_std, st_indic );
                    end
                else
                    y = 1.;
                end
                
            case { 'VMM-NH3' }
                
                ref_level = 1.5;
                
                % See remarks above : VAR(log(1+X)) = VAR(X)/(1+X)^2
                if cnf.Option.logtrans
                    ref_level = ref_level ./ ( 1 + avg_ref );
                end
                
                y = ref_level ./ (cnf.p_std(1)*st_indic + cnf.p_std(2));
                
            otherwise
                error( 'rio_stdtrend:: deployment not defined in this function!' );
        end
        
    otherwise
        % =====================================================================
        % Configuration for version 3.6 and onwards
        % Bino Maiheu, 2013-10-21
        % =====================================================================
        if ~isfield( cnf, 'stdTrend' )
            error( 'We expect the avgtrend in the XML configuration for this version...' );
        end
        
        ref_level = cnf.stdTrend.ref_level;
        
        % See remarks above : VAR(log(1+X)) = VAR(X)/(1+X)^2
        if cnf.Option.logtrans
            ref_level = ref_level ./ ( 1 + avg_ref );
        end
        
        if ~isempty(st_indic)
            switch( cnf.stdTrend.type )
                case 'poly2'
                    yy = rio_poly2( cnf.p_std, st_indic, cnf.stdTrend.indic_lo, cnf.stdTrend.indic_hi );
                case 'poly1'
                    yy = rio_poly1( cnf.p_std, st_indic, cnf.stdTrend.indic_lo, cnf.stdTrend.indic_hi );
                otherwise
                    error( 'Invalid trend type : %s', cnf.stdTrend.type );
            end
            y  = ref_level ./ yy;
        else
            y = 1.;
        end
        
end

end
