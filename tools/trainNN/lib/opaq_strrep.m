% OPAQ_STRREP translate the string patterns in the config file
%
% str = opaq_strrep( str, 'station', '40ML01', 'fc_hor', 3 )
%
% Possble wildcards in the path specifications are :  
%  'station', 'name'   --> @station@ : will be replaced by the station name
%  'pol', 'pol_name'   --> @pol@     : will be replaced by the pollutant name
%  'agg_str', '...'    --> @agg_str@ : will be replaced by the aggregation string
%  'model', 'name'     --> @model@   : will be replaced by the model name
%  'mor_agg', 9        --> @mor_agg@ : will be replaced by a string having the current mor_agg
%  'fc_hor', 3         --> @fc_hor@  : will be replaced by day0, day1, day2 etc...				
%
% Author :  Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function s = opaq_strrep( s, varargin )

p = inputParser;
p.addParameter( 'station', '', @isstr );
p.addParameter( 'pol', '', @isstr );
p.addParameter( 'agg_str', '', @isstr );
p.addParameter( 'model', '', @isstr );
p.addParameter( 'mor_agg', '', @isnumeric );
p.addParameter( 'fc_hor', '', @isnumeric );
p.parse( varargin{:} );

fields = fieldnames(p.Results);
for k=1:length(fields)
    
    switch fields{k}
        case 'mor_agg'
            s = strrep( s, [ '@' fields{k} '@' ], sprintf( '%02dUT', p.Results.( fields{k} ) ) );
        case 'fc_hor'
            s = strrep( s, [ '@' fields{k} '@' ], sprintf( 'day%d', p.Results.( fields{k} ) ) );
        otherwise
            s = strrep( s, [ '@' fields{k} '@' ], p.Results.( fields{k} ) );
    end
end
