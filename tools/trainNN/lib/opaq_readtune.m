% OPAQ_READTUNE Reads the tune from the parsed xml tune configuration
%
%   tune = opaq_readtune( xml, mode, pol_name, agg_str, st_name, fc_hor )
%
% In the above : xml is a structure which is created by : 
%
%   xml = parseChildNodes( xmlread( tune_file ) )
%
% the function returns a structure with the tune : 
%
%   tune.rtc_mode
%   tune.rtc_param
%   tune.model_name
%
% Author : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function tune = opaq_readtune( xml, mode, pol_name, agg_str, st_name, fc_hor )

tune = struct();

% check the attributes
if ~strcmp( xml.Attributes( strcmp( {xml.Attributes.Name}, 'pollutant' ) ).Value, pol_name ) || ...
   ~strcmp( xml.Attributes( strcmp( {xml.Attributes.Name}, 'aggr' ) ).Value, agg_str ) || ...
   ~strcmp( xml.Attributes( strcmp( {xml.Attributes.Name}, 'mode' ) ).Value, mode )
 error( 'OPAQ:NotFound', 'Tune file doesnt match requested pollutajnt/aggr/mode' );
end

stNodes = xml.Children( strcmp( { xml.Children.Name }, 'station' ) );
have_st = false;
for k=1:length(stNodes)
   if strcmp( stNodes(k).Attributes( strcmp( {stNodes(k).Attributes.Name}, 'name') ).Value, st_name )        
       have_st = true;
       break;
   end
end

if ~have_st
    error( 'OPAQ:NotFound', 'Station not found : %s', st_name );
end

modelNodes = stNodes(k).Children( strcmp( { stNodes(k).Children.Name }, 'model' ) );
for k=1:length(modelNodes)
   if str2num( modelNodes(k).Attributes( strcmp( {modelNodes(k).Attributes.Name}, 'fc_hor') ).Value ) == fc_hor
       
       tune.rtc_mode   = str2num( modelNodes(k).Attributes( strcmp( {modelNodes(k).Attributes.Name}, 'rtc_mode') ).Value );
       tune.rtc_param  = str2num( modelNodes(k).Attributes( strcmp( {modelNodes(k).Attributes.Name}, 'rtc_param') ).Value );
       tune.model_name = modelNodes(k).Children(1).Data;
       return;
   end
end

error( 'OPAQ:NotFound', 'Forecast horizon day%d not found for station %s', fc_hor, st_name );


