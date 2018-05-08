classdef dayavg_full_meteo < opaqmodels.opaq_model
   methods
       function obj = dayavg_full_meteo( varargin )
           obj = obj@opaqmodels.opaq_model( varargin{:} );
           
           obj.name = 'dayavg_full_meteo';
           %obj.size = 12;
           obj.size = 36;
                      
       end
    
       function names = input_names( obj )
           names = { 'CAVG_DAY0', 'CAVG_DAYM1', ...
                     'BLH_DAYN', ...
                     'CC_DAYNM1_DAYN', ...
                     'U10_DAYNM1_DAYN', 'V10_DAYNM1_DAYN', ...
                     'U30_DAYNM1_DAYN', 'V30_DAYNM1_DAYN', ...
                     'T2M_DAYN', 'DT30M_DAYN', 'RH_DAYN', ...
                     'WEEKEND', ...
                     'P8', 'P9', 'P10', 'P11', 'P12', 'P13', 'P14', 'P15', 'P16', ...
                     'P17', 'P18', 'P19', 'P20', 'P21', 'P22', 'P23', 'P25', ...
                     'P26', 'P27', 'P28', 'P29', 'P30', 'P31', 'P32' };
       end
       
       function tf = log_trans( obj )          
           tf = ones( 1, obj.size );
           tf( 4:end ) = 0; % these have not been log transformed
       end
       
       
       function [ target, input, have_sample ] = make_sample( obj, fc_date, fc_hor )
           
           pBLH    = 7;  % boundary layer height
           pCC     = 14; % medium cloud cover
           pWDIR10 = 4;  % 10 m wind direction
           pWSP10  = 3;  % 10 m wind speed
           pWDIR30 = 6;  % 30 m wind direction
           pWSP30  = 5;  % 30 m wind speed
           pT2M    = 1;  % 2 m temperature
           pT30M   = 2;  % 3 m temperature
           pRH     = 24; % RH in layer 0 - 50 m
                      
           
           target = [];
           input  = [];
           have_sample = true;
           
           if ~isscalar( fc_date ) || ~isscalar( fc_hor )
               error( 'make sample only for scalars ...' );
           end
                                 
           iN = find( obj.xx_obs.xx_days == fc_date );           
           
           if isempty( iN ) 
               have_sample = false;
               return;
           end
           
           day0   = fc_date - fc_hor;
           daym1  = fc_date - fc_hor - 1;
           dayN   = fc_date;
           dayNm1 = fc_date - 1;
           
           % -- the target sample : daily averaged concentrations of dayN
           target = log( 1 + obj.xx_obs.xx_dayavg( iN ) );
           
           % -- the first input sample : averaged concentrations of day0 up to
           %    a certain threshold : 9 UTC here..
           idx = find( obj.xx_obs.xx_hours >= day0 & obj.xx_obs.xx_hours < ( day0 + datenum( 0,0,0,9,0,0 ) ) );
           [ have_sample, xx ] = ovl_mean( obj.xx_obs.xx_value( idx ), -9999, have_sample );
           input(1) = log( 1 + xx );
           
           
           % -- the first input sample : averaged concentrations of day-1           
           idx = find( obj.xx_obs.xx_hours >= daym1 & obj.xx_obs.xx_hours < day0 );
           [ have_sample, xx ] = ovl_mean( obj.xx_obs.xx_value( idx ), -9999, have_sample );
           input(2) = log( 1 + xx );
                      
           
           % -- daily average boundary layer height
           idx = find( obj.xx_meteo(:,1) >= dayN & obj.xx_meteo(:,1) < dayN+1 );
           [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pBLH ), -999, have_sample );
           input(3) = log( 1 + xx );
           
           
           % select time stamps for day N-1 (12h-21h) until day N (0h->9h)   
           idx = find( obj.xx_meteo(:,1) >= ( dayNm1 + datenum(0,0,0,12,0,0) ) & ...
                       obj.xx_meteo(:,1) <  ( dayN + datenum( 0,0,0,12,0,0 ) ) );
           
           % -- medium cloud cover
           [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pCC ), -999, have_sample );
           input(4) = xx;  
           
           % -- winddirection & wind speed at 10 m
           [ have_sample, x_vec, y_vec ] = ovl_winddir( obj.xx_meteo( idx, 1+pWDIR10 ), -999, have_sample );
           [ have_sample, wind_speed ]   = ovl_mean( obj.xx_meteo( idx, 1+pWSP10 ), -999, have_sample );
                       
           input(5) = x_vec * wind_speed;
           input(6) = y_vec * wind_speed;
           
           
            % -- winddirection & wind speed at 30 m
           [ have_sample, x_vec, y_vec ] = ovl_winddir( obj.xx_meteo( idx, 1+pWDIR30 ), -999, have_sample );
           [ have_sample, wind_speed ]   = ovl_mean( obj.xx_meteo( idx, 1+pWSP30 ), -999, have_sample );
                       
           input(7) = x_vec * wind_speed;
           input(8) = y_vec * wind_speed;
                  
           
           % -- temperature & humidity of day N
           idx = find( obj.xx_meteo(:,1) >= dayN & obj.xx_meteo(:,1) < dayN+1 );
           [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pT2M ), -999, have_sample );
           input(9) = xx;
                      
           [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pT30M ), -999, have_sample );
           input(10) = xx - input(9); % temperature difference with 2m
                      
           [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pRH ), -999, have_sample );
           input(11) = xx;
           
           % -- week/weekend ?
           if ( weekday( fc_date ) == 1 || weekday( fc_date ) == 7 )
                input(12) = 1;
            else
                input(12) = 0;
           end           
                      
           % now add the rest of the parameters to look for any other
           % correlations... let's keep it at dayN
           p = [ 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 25 26 27 28 29 30 31 32 ];
           for ii=1:length(p)
               [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+p(ii) ), -999, have_sample );
               input(12+ii) = xx;
           end
           
           
       end
       
   end
    
end