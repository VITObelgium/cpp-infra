classdef model1 < opaqmodels.opaq_model
   methods
       function obj = model1( varargin )
           obj = obj@opaqmodels.opaq_model( varargin{:} );
           
           obj.name = 'model1';
           obj.size = 2;           
                      
       end
    
       function names = input_names( obj )
           names = { 'CAVG_DAY0', 'BLH_DAYN' };                     
       end
       
       function tf = log_trans( obj )          
           tf = ones( 1, obj.size );       
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
           idx = find( obj.xx_obs.xx_hours >= day0 & obj.xx_obs.xx_hours < ( day0 + datenum( 0,0,0,obj.mor_agg,0,0 ) ) );
           [ have_sample, xx ] = ovl_mean( obj.xx_obs.xx_value( idx ), -9999, have_sample );
           input(1) = log( 1 + xx );
           
           
           % -- daily average boundary layer height
           idx = find( obj.xx_meteo(:,1) >= dayN & obj.xx_meteo(:,1) < dayN+1 );
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
           input(2) = log( 1 + xx );
           
           
       end
       
   end
    
end