classdef model3 < opaqmodels.opaq_model
   methods
       function obj = model3( varargin )
           obj = obj@opaqmodels.opaq_model( varargin{:} );
           
           obj.name = 'model3';
           obj.size =  11;                                
       end
    
       function names = input_names( obj )
           names = { ...
               'CAVG_DAY0', ...
               'CAVG_DAYM1', ...
               'BLH_DAYN', ...
               'CC_DAYN', ...               
               'T2M_DAYN', ...             
               'RH_DAYN', ...
               'S_DAYN', ...
               'HTRANSP_DAYN', ...
               'U10_DAYNM1_DAYN', ...
               'V10_DAYNM1_DAYN', ...
               'WEEKEND' };           
       end
       
       function tf = log_trans( obj )          
           tf = [ ...
               1; ...  
               1; ...
               1; ...
               0; ...
               0; ...
               0; ...
               0; ...
               1; ...
               0; ...
               0; ...
               0 ];
       end       
       
       function [ target, input, have_sample ] = make_sample( obj, fc_date, fc_hor )
           
           pBLH    = 7;  % boundary layer height
           pCC     = 13; % low cloud cover
           pWDIR10 = 4;  % 10 m wind direction
           pWSP10  = 3;  % 10 m wind speed
           pT2M    = 1;  % 2 m temperature
           pT30M   = 2;  % 3 m temperature
           pRH     = 24; % RH in layer 0 - 50 m
           pS      = 21; % Buyltinck Malet lambda parameter       
           pTransp = 16; % horizontal transport in boundary layer : BLH x mean wind in BLH
                      
           
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
           switch( obj.aggr_str )
               case 'dayavg'     
                   if isnan( obj.xx_obs.xx_dayavg( iN ) ) || ( obj.xx_obs.xx_dayavg( iN ) < 0 )
                       have_sample = false;
                   else
                       target = log( 1 + obj.xx_obs.xx_dayavg( iN ) );
                   end
               case 'max1h'
                   if isnan( obj.xx_obs.xx_max1h( iN ) ) || ( obj.xx_obs.xx_max1h( iN ) < 0 )
                       have_sample = false;
                   else
                       target = log( 1 + obj.xx_obs.xx_max1h( iN ) );
                   end
               case 'max8h'
                   if isnan( obj.xx_obs.xx_max8h( iN ) ) || ( obj.xx_obs.xx_max8h( iN ) < 0 )
                       have_sample = false;
                   else
                       target = log( 1 + obj.xx_obs.xx_max8h( iN ) );
                   end
               otherwise
                   error( 'aggregation is not supported...' );
           end

           % -- the first input sample : averaged concentrations of day0 up to
           %    a certain threshold : 9 UTC here..
           idx = find( obj.xx_obs.xx_hours >= day0 & obj.xx_obs.xx_hours < ( day0 + datenum( 0,0,0,9,0,0 ) ) );
           [ have_sample, xx ] = ovl_mean( obj.xx_obs.xx_value( idx ), -9999, have_sample );
           input(1) = log( 1 + xx );
                      
           % -- the first input sample : averaged concentrations of day-1           
           idx = find( obj.xx_obs.xx_hours >= daym1 & obj.xx_obs.xx_hours < day0 );
           [ have_sample, xx ] = ovl_mean( obj.xx_obs.xx_value( idx ), -9999, have_sample );
           input(2) = log( 1 + xx );
                      
           
           % --------------------------------------------------------------
           %  The meteo parameters, averages of day N
           % --------------------------------------------------------------
           idx = find( obj.xx_meteo(:,1) >= dayN & obj.xx_meteo(:,1) < dayN+1 );           
           
           % -- boundary layer height
           if strcmp( obj.aggr_str, 'max1h' )
               if strcmp( obj.pol_name, 'o3' )
                   [ have_sample, xx ] = ovl_max( obj.xx_meteo( idx, 1+pBLH ), -999, have_sample );
               else
                   [ have_sample, xx ] = ovl_min( obj.xx_meteo( idx, 1+pBLH ), -999, have_sample );
               end
           else
               % also in case of max8h we use daily averages since the timeseries is a lot smooother               
               [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pBLH ), -999, have_sample );
           end
           input(3) = log( 1 + xx );
                      
           
           % -- medium cloud cover (P14), average dag N
           [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pCC ), -999, have_sample );
           input(4) = xx;  
           
           
           % -- temperature of day N
           if strcmp( obj.aggr_str, 'max1h' )
               if strcmp( obj.pol_name, 'o3' )
                   [ have_sample, xx ] = ovl_max( obj.xx_meteo( idx, 1+pT2M ), -999, have_sample );
               else
                   [ have_sample, xx ] = ovl_min( obj.xx_meteo( idx, 1+pT2M ), -999, have_sample );
               end
           else
                [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pT2M ), -999, have_sample );
           end
           input(5) = xx;
           
           
           % -- relative humidity
           if strcmp( obj.aggr_str, 'max1h' )
               if strcmp( obj.pol_name, 'o3' )                  
                   [ have_sample, xx ] = ovl_min( obj.xx_meteo( idx, 1+pRH ), -999, have_sample );
               else                   
                   [ have_sample, xx ] = ovl_max( obj.xx_meteo( idx, 1+pRH ), -999, have_sample );
               end
           else
               [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pRH ), -999, have_sample );
           end
           input(6) = xx;
                                 
           
           % -- buyltinck malet S parameter
           if strcmp( obj.aggr_str, 'max1h' )
               if strcmp( obj.pol_name, 'o3' )                  
                   [ have_sample, xx ] = ovl_min( obj.xx_meteo( idx, 1+pS ), -999, have_sample );
               else
                   [ have_sample, xx ] = ovl_max( obj.xx_meteo( idx, 1+pS ), -999, have_sample );
               end
           else
               [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pS ), -999, have_sample );
           end
           input(7) = xx; 
                      
           
           % -- horizontal transport in boundary layer (BLH * mean BL winspeed)
           if strcmp( obj.aggr_str, 'max1h' )
               if strcmp( obj.pol_name, 'o3' )                  
                   [ have_sample, xx ] = ovl_max( obj.xx_meteo( idx, 1+pTransp ), -999, have_sample );
               else
                   [ have_sample, xx ] = ovl_min( obj.xx_meteo( idx, 1+pTransp ), -999, have_sample );
               end
           else
               [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+pTransp ), -999, have_sample );
           end
           input(8) = log( 1 + xx );
           
           
           % --------------------------------------------------------------
           %  The meteo parameters, select time stamps for day N-1 (12h-21h) 
           %  until day N (0h->9h)   
           % --------------------------------------------------------------            
           idx = find( obj.xx_meteo(:,1) >= ( dayNm1 + datenum(0,0,0,12,0,0) ) & ...
                       obj.xx_meteo(:,1) <  ( dayN + datenum( 0,0,0,12,0,0 ) ) );
           
           
           % -- winddirection & wind speed at 10 m
           [ have_sample, x_vec, y_vec ] = ovl_winddir( obj.xx_meteo( idx, 1+pWDIR10 ), -999, have_sample );
           [ have_sample, wind_speed ]   = ovl_mean( obj.xx_meteo( idx, 1+pWSP10 ), -999, have_sample );
                       
           input(9) = x_vec * wind_speed;
           input(10) = y_vec * wind_speed;
                                        
           
           % -- week/weekend ?
           if ( weekday( fc_date ) == 1 || weekday( fc_date ) == 7 )
                input(11) = 1;
            else
                input(11) = 0;
           end                                                       
                      
       end
       
   end
    
end