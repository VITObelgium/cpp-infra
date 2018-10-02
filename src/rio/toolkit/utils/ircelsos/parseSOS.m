function [ xx_date, xx_value, st_info ] = parseSOS( xml )

xx_date  = [];
xx_value = [];
st_info  = [];

% -- Retrieve some element nodes
if ~strcmp( xml.Name, 'om:ObservationCollection' ), error( 'xml root tag not recognized' ); end
memNode = xml.Children( strcmp( {xml.Children.Name}, 'om:member' ) );
if length( memNode ) ~= 1, error( 'no/more than one om:member' ); end
obsNode = memNode.Children( strcmp( {memNode.Children.Name}, 'om:Observation' ) );
if length( obsNode ) ~= 1, error( 'no/more than one om:Observation' ); end

% -- decode the feature information
featNode = obsNode.Children( strcmp( {obsNode.Children.Name}, 'om:featureOfInterest' ) );
if length( featNode ) ~= 1, error( 'no/more than one om:featureOfInterest' ); end
pointNode = featNode.Children( strcmp( {featNode.Children.Name}, 'sa:SamplingPoint' ) );
if length( pointNode ) ~= 1, error( 'no/more than one sa:SamplingPoint' ); end
try
  st_info.type= pointNode.Children( strcmp( {pointNode.Children.Name}, 'gml:description') ).Children.Data;
  name        = pointNode.Children( strcmp( {pointNode.Children.Name}, 'gml:name') ).Children.Data;
  [ s1, s2 ] = strtok( name, '-' );
  st_info.name = strtrim(s1);
  st_info.long_name = strtrim(s2);
  st_info.id          = pointNode.Attributes( strcmp( {pointNode.Attributes.Name}, 'gml:id' ) ).Value;
  
  posNode = pointNode.Children( strcmp( {pointNode.Children.Name}, 'sa:position' )  );
  pNode   = posNode.Children( strcmp( {posNode.Children.Name}, 'gml:Point' )  );
  ppNode  = pNode.Children( strcmp( {pNode.Children.Name}, 'gml:pos' )  );
  
  coords = str2num(ppNode.Children.Data);
  st_info.x   = coords(1);
  st_info.y   = coords(2);
  st_info.crs = ppNode.Attributes( strcmp( {ppNode.Attributes.Name}, 'srsName' ) ).Value;
catch ME
  error( 'error decoding sampling point' );
end

% -- decode the data array values
resNode = obsNode.Children( strcmp( {obsNode.Children.Name}, 'om:result' ) );
if length( resNode ) ~= 1, error( 'no/more than one om:result' ); end
dataNode = resNode.Children( strcmp( {resNode.Children.Name}, 'swe:DataArray' ) );
if length( dataNode ) ~= 1,  error( 'no/more than one om:result' ); end

% decode element count
try
  elcNode = dataNode.Children( strcmp( {dataNode.Children.Name}, 'swe:elementCount' ) );
  if length( elcNode ) ~= 1,  error( 'no/more than one swe:elementCount in we:DataArray' ); end
  cNode   = elcNode.Children( strcmp( {elcNode.Children.Name}, 'swe:Count' ) );
  if length( cNode ) ~= 1,  error( 'no/more than one swe:Count in we:elementCount' ); end
  n = str2double( cNode.Children( strcmp( {cNode.Children.Name}, 'swe:value' ) ).Children.Data );
catch ME
  error( 'error decoding element count' );
end

% decode components and encoding
try
  compNode = dataNode.Children( strcmp( {dataNode.Children.Name}, 'swe:elementType' ) );
  recNode  = compNode.Children( strcmp( {compNode.Children.Name}, 'swe:DataRecord' ) );
  
  ifld = find( strcmp( {recNode.Children.Name}, 'swe:field' ) );
  if length( ifld ) ~= 2
    error( 'number of component fields is not 2' );
  end
  field1 = recNode.Children(ifld(1)).Attributes(strcmp( {recNode.Children(ifld(1)).Attributes.Name}, 'name' ) ).Value;
  field2 = recNode.Children(ifld(2)).Attributes(strcmp( {recNode.Children(ifld(2)).Attributes.Name}, 'name' ) ).Value;
  
  if ~strcmp( field1, 'SamplingTime' ),
    error( 'first field in component is not SamplingTime' );
  end
  
catch ME
  error( 'error decoding component node' );
end

try
  encNode  = dataNode.Children( strcmp( {dataNode.Children.Name}, 'swe:encoding' ) );
  txtBlock = encNode.Children( strcmp( {encNode.Children.Name}, 'swe:TextBlock' ) );
  decimalSep = txtBlock.Attributes( strcmp( {txtBlock.Attributes.Name}, 'decimalSeparator' ) ).Value;
  tokenSep   = txtBlock.Attributes( strcmp( {txtBlock.Attributes.Name}, 'tokenSeparator' ) ).Value;
  blockSep   = txtBlock.Attributes( strcmp( {txtBlock.Attributes.Name}, 'blockSeparator' ) ).Value;
catch ME
  error( 'error decoding encoding node' );
end

if decimalSep ~= '.'
  error( 'unsupported decimal symbol' );
end

% get the actual data & decode it, assume first field is sampling time
% we have already checked this...
valsNode = dataNode.Children( strcmp( {dataNode.Children.Name}, 'swe:values' ) );
remain   = valsNode.Children.Data;
while true
   [str, remain] = strtok(remain, blockSep);
   if isempty(str),  break;  end
   
   %fprintf('%s\n', str);  
   
   % here we assume we only have 2 fields, and the first field is the 
   % timestamp... we have checked this above...
   [strTime, strValue ] = strtok( str, tokenSep );
   
   local_date = datenum( strTime(1:end-6), 'yyyy-mm-ddTHH:MM:SS.FFF' );
   dv         = datevec( datenum( strTime(end-4:end), 'HH:MM' ) );
   offset     = datenum(0,0,0,dv(4),dv(5),0);
   sign       = strTime(end-5);
   switch sign
     case '+', xx_date    = [ xx_date; local_date-offset ];
     case '-', xx_date    = [ xx_date; local_date+offset ];
     otherwise
       error( 'unknown sign, error decoding timestamp' );
   end
   xx_value   = [ xx_value ; str2double(strValue) ];
end

