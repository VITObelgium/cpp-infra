% OPAQ_MODEL
%
% An abstract base class for an OPAQ regression model
% the pure virtual method make_sample craetes an input sample
% given a forecast date and the foreacst horizon
classdef opaq_model
    
    properties ( GetAccess = 'public', SetAccess = 'protected' )
        name = '';
        size = 0;        
    end
    
    properties ( GetAccess = 'protected', SetAccess = 'private' )
        pol_name;
        aggr_str;   
        mor_agg;
        xx_obs;
        xx_meteo;
    end
    
    methods
        % constructor
        function obj = opaq_model( varargin )
            if nargin == 5
                obj.pol_name  = varargin{1};
                obj.aggr_str  = varargin{2};
                obj.mor_agg   = varargin{3};
                obj.xx_obs    = varargin{4};
                obj.xx_meteo  = varargin{5};
            else
                obj.pol_name = '';
                obj.aggr_str = 'dayavg';
                obj.mor_agg  = 9;
                obj.xx_obs   = [];
                obj.xx_meteo = [];                
            end
        end
        
    end
        
    methods (Abstract = true )
        
        % abstract method to create a sample
        [ target_sample, input_sample, have_sample ] = make_sample( obj, fc_date, fc_hor );
        
        % abstract method to return the names of the variables in the input
        % vector
        names = input_names( obj );
        
        % abstract method to return whether the input sample has been 
        % log transformed
        tf = log_trans( obj );
        
    end
    
end
