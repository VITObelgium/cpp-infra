classdef all_meteo < opaqmodels.opaq_model
   methods
       function obj = all_meteo( varargin )
           obj = obj@opaqmodels.opaq_model( varargin{:} );
           
           obj.name = 'all_meteo';
           obj.size = 32;           
                      
       end
    
       function names = input_names( obj )
           names = cell(obj.size,1);
           for k=1:32
               names{k} = sprintf( 'P%02d', k );
           end
       end
       
       function tf = log_trans( obj )          
           tf = zeros( 1, obj.size );           
       end
       
       
       function [ target, input, have_sample ] = make_sample( obj, fc_date, fc_hor )                                
           
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
                   target = log( 1 + obj.xx_obs.xx_dayavg( iN ) );
               case 'max1h'
                   target = log( 1 + obj.xx_obs.xx_max1h( iN ) );
               case 'max8h'
                   target = log( 1 + obj.xx_obs.xx_max8h( iN ) );
               otherwise
                   error( 'aggregation is not supported...' );
           end           
                                           
           % -- the all meteo simply averages of day N...
           idx = find( obj.xx_meteo(:,1) >= dayN & obj.xx_meteo(:,1) < dayN+1 );
           for ii=1:obj.size
               [ have_sample, xx ] = ovl_mean( obj.xx_meteo( idx, 1+ii ), -999, have_sample );
               input(ii) = xx;
           end
           
           % always return true for the meteo model... 
           have_sample = true; 
           
       end
       
   end
    
end