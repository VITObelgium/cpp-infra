% OPAQ_READFFNET Reads in an OPAQ XML network and returns matlab object
%
%   [ net, input_PS, target_PS ] = opaq_readffnet( fname )
%
% This routine reads in an OPAQ XML neural network file and returns a
% matlab net object
%
% See also opaq_writeffnet
%
% Author Bino Maiheu, (c) VITO 2014 - bino.maiheu@vito.be

function [ net, input_PS, target_PS ] = opaq_readffnet( fname )

xml = parseXML( fname );

% reading normalisation
input_node  = find( strcmp( {xml.Children.Name}, 'input' ) );
output_node = find( strcmp( {xml.Children.Name}, 'output' ) );

nr_inp  = str2double( getXMLAttrib( xml.Children(input_node), 'size' ) );
nr_targ = str2double( getXMLAttrib( xml.Children(output_node), 'size' ) );

input_scaler_node  = find( strcmp( {xml.Children(input_node).Children.Name}, 'scaler' ) );
output_scaler_node = find( strcmp( {xml.Children(output_node).Children.Name}, 'scaler' ) );

xml_input_scaler = xml.Children(input_node).Children(input_scaler_node);
xml_output_scaler = xml.Children(output_node).Children(output_scaler_node);

fprintf( 'Input scaler type : %s\n', getXMLAttrib( xml_input_scaler, 'type' ) );
fprintf( 'Output scaler type : %s\n', getXMLAttrib( xml_output_scaler, 'type' ) );

input_PS = read_scaler( nr_inp, xml_input_scaler );
target_PS = read_scaler( nr_targ, xml_output_scaler );

disp(input_PS)
disp(target_PS)

% construct network
layer_nodes = find( strcmp( {xml.Children.Name}, 'layer' ) );

nr_lyr = numel(layer_nodes);

% manually construct a feed forward network
% has some limitations
net = network;
net.numInputs     = 1; % one input vector, size is given below !!
net.numLayers     = nr_lyr;
net.biasConnect   = ones(nr_lyr,1);
net.inputConnect  = [ 1; zeros(nr_lyr-1,1) ];
net.layerConnect  = zeros(nr_lyr); 
net.layerConnect(2:end,1:end-1) = eye(nr_lyr-1);  % layer i has weight coming from layer i-1 always
net.outputConnect = [ zeros(1,nr_lyr-1) 1 ];

% set network inputs, we only have 1 input (of size nr_inp)
net.inputs{1}.size = nr_inp; % nr input parameters for input layer

% here we set the layer sizes and transfer functions
% note that in our implementation of the XML, the weights are
for i=1:nr_lyr
    lyr_node = xml.Children(layer_nodes(i));
    
    % just to be sure...
    lyr_id = str2double( getXMLAttrib( lyr_node, 'id' ) ); 
    if lyr_id ~= i,
        error( 'XML layer sequence corrupted...\n' );
    end
    
    % parse the layer element        
    lyr_name = getXMLAttrib( lyr_node, 'name' );      
    fprintf( 'Parsing layer : %s\n', lyr_name );
    
    % Transfer function
    net.layers{i}.transferFcn = getXMLAttrib( xml.Children(layer_nodes(i)), 'transfcn' );
    fprintf( 'Transfer function set to : %s\n', net.layers{i}.transferFcn );
    
    % Layer size
    net.layers{i}.size = str2double( getXMLAttrib( xml.Children(layer_nodes(i)), 'size' ) );     
    bias_idx = find( strcmp( {lyr_node.Children.Name}, 'bias' ) );
        
    % Layer bias
    net.b{i} = str2num(lyr_node.Children(bias_idx).Children.Data)';
    
    % Layer weights
    w_idx = find( strcmp( {lyr_node.Children.Name}, 'weights' ) );
    W = str2num(lyr_node.Children(w_idx).Children.Data);
    
    % set in matlab network object
    if i==1, 
       % first layer weight in the input file is in matlab the IW
       % only 1 input so this is {1,1}
       net.IW{1,1} = W;
    else
        % set layer weights : to go from the previous layer (i-1) to this
        % layer (i)
        net.LW{i,i-1} = W;
    end
end


function map_PS = read_scaler( n, xml )

type = getXMLAttrib( xml, 'type' );

switch lower(type)
  case 'mapstd'
    i_xmean = find( strcmp( {xml.Children.Name}, 'xmean' ) );
    i_xstd  = find( strcmp( {xml.Children.Name}, 'xstd' ) );
    
    mean_p = str2num(xml.Children(i_xmean).Children.Data)';
    std_p  = str2num(xml.Children(i_xstd).Children.Data)';
    
    % construct the mapstd structure
    [ dummy, map_PS ] = mapstd([]);
    map_PS.xrows = n;
    map_PS.yrows = n;
    map_PS.xmean = mean_p;
    map_PS.xstd  = std_p;
    map_PS.no_change = 0;
    
  case 'minmax'
    
    error( 'Not implemented yet here...' );
    
  otherwise
    error( 'Undefined scaler type %s', type );
end
