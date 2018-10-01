%OPAQ_WRITEFFNET Writes a matlab neural network object to OPAQ XML format
%
%  opaq_writeffnet( fname, net, input_PS, target_PS )
%
% This function takes a network object and writes it in the XML format
% used in OPAQ to the output. 
% The input/output scaler is autmatically determined from the fieldnames
% of the input_PS and target_PS normalization structures. 
%
% Note that for whatever bizarr reason this routine may end up with files
% containing \x0 (0x0, or nul) symbols, which are not allowed in XML.
% Fix suing : 
%
%     sed -i 's/\x0//g' file.xml
%
%
% See also opaq_readffnet
%
% Author Bino Maiheu, (c) VITO 2014 - bino.maiheu@vito.be

function opaq_writeffnet( fname, net, input_PS, target_PS )

if net.numInputs ~= 1
    error( 'Only for networks with a single input vector...' );
end

% -- create doc node
docNode = com.mathworks.xml.XMLUtils.createDocument('feedforwardnet');
docEl   = docNode.getDocumentElement;

% -- create input element and append as child to docNode
inp = docNode.createElement( 'input' );
inp.setAttribute( 'size', num2str(net.inputs{1}.size) );

% -- now add the scalers
inpSc = genScalerElement( docNode, input_PS );
inp.appendChild(inpSc);

docEl.appendChild(inp);

% -- write the layers
for i=1:net.numLayers
    lyr = docNode.createElement( 'layer' );
    
    % -- layer id
    lyr.setAttribute( 'id', num2str(i) );
    
    % -- layer size
    lyr.setAttribute( 'size', num2str(net.layers{i}.size) );
    
    % -- layer type : hidden or output
    if i<net.numLayers
        name = 'hidden';
    else
        name = 'output';
    end
    lyr.setAttribute( 'name', name );
    
    % -- transfer function
    lyr.setAttribute( 'transfcn', net.layers{i}.transferFcn );
        
    if i==1
        W = net.IW{1,1}; % input weight
    else
        W = net.LW{i,i-1}; % layer weight
    end
    b = net.b{i};
    
    % -- convert to string
    W_str = num2str(W, '%.8f '); 
    lines=size(W_str,1);
    W_str = [W_str char(sprintf('\n')*[ones(lines-1,1);0])]; % add line breaks after each line
    W_out = [];
    for k=1:lines
        W_out = [ W_out W_str(k,:) ];
    end
    
    if ~isvector(b)
        error('Biasses are not a single vector... please check ');
    end
    % -- this is only a single vector, transpose though
    b_str = num2str(b', '%.8f ');
    
    % weights
    wEl = docNode.createElement('weights');        
    wEl.appendChild( docNode.createTextNode( W_out ) );
    lyr.appendChild(wEl);

    % biasses
    bEl = docNode.createElement('bias');
    bEl.appendChild( docNode.createTextNode( b_str ) );
    lyr.appendChild(bEl);
    
    % -- append the layer to the file
    docEl.appendChild(lyr);    
end


% -- create output element and append as child to docNode
outp = docNode.createElement( 'output' );
outp.setAttribute( 'size', num2str(net.layers{end}.size) );
outpSc = genScalerElement( docNode, target_PS );
outp.appendChild(outpSc);
docEl.appendChild(outp);

% -- write the file
xmlwrite( fname, docNode );


function sc = genScalerElement( docNode, PS )

sc = docNode.createElement('scaler');
if isfield( PS, 'xmean' ) && isfield( PS, 'xstd' )
    if PS.ymean ~= 0 || PS.ystd ~= 1
        error( 'Currently OPAQ only supports mapstd to 0/1' );
    end
    sc.setAttribute( 'type', 'mapstd' );
    
    el = docNode.createElement('xmean');        
    el.appendChild( docNode.createTextNode( num2str( PS.xmean', '%.8f ' ) ) );
    sc.appendChild(el);
    
    el = docNode.createElement('xstd');        
    el.appendChild( docNode.createTextNode( num2str( PS.xstd', '%.8f ' ) ) );
    sc.appendChild(el);
    
elseif isfield( PS, 'xmin' ) && isfield( PS, 'xmax' )
    inpSc.setAttribute( 'type', 'mapminmax' );
    if PS.ymin ~= -1 || PS.ymax ~= 1
        error( 'Currently OPAQ only supports mapminmax to -1/1' );
    end
    sc.setAttribute( 'type', 'mapminmax' );
    
    el = docNode.createElement('xmin');
    el.appendChild( docNode.createTextNode( num2str( PS.xmin', '%.8f ' ) ) );
    sc.appendChild(el);
    
    el = docNode.createElement('xmax');        
    el.appendChild( docNode.createTextNode( num2str( PS.xmax', '%.8f ' ) ) );
    sc.appendChild(el);
    
else
    error( 'Unsupported PS, could not found either xmean/xstd or xmin/xmax' );
end

