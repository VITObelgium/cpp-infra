% ovl2opaq - convert OVL neural network files to OPAQ format
%
% Macro to read in the OVL neural network text files and export them to
% OPAQ XML format
% 
% Bino Maiheu (c) VITO 2014 - bino.maiheu@vito.be

addpath src
addpath 'D:\Projects\N78H9\Deployments\Yangzhou\ovl_yangzhou-201403\ovl-v2014\lib';

%% Configuration

% -- root folder for the ovl sources
inputdir = 'D:\Projects\N78H9\Deployments\Yangzhou\ovl_yangzhou_RMSE_0-5';

% -- what architecture to export ( subfolder of the input dir )
arch     = 'arch_RMSE_7CST';

% -- output folder for the OPAQ networks
outputdir = 'networks';

% -- define list of pollutants, models and fc_hors to export
pol_name   = { 'pm10', 'pm25', 'no2', 'o3', 'so2', 'co' };
model_name = { 'model-1-7CST', 'model-2-7CST', 'model-3-7CST' };
fc_hors    = 0:5;

%% Here we go...
for k=1:length( pol_name )
    
    cnf = ovl_config( pol_name{k} );
    
    for l=1:length( model_name )
        
        % in OPAQ group networks per model
        out_base = fullfile( outputdir, model_name{l}, pol_name{k} );
        if ~exist( out_base, 'dir' )
            mkdir( out_base )
        end
        
        for m=1:length(fc_hors)
            
            nn_base = fullfile( inputdir, arch, pol_name{k}, model_name{l}, ...
                sprintf( 'day%d', fc_hors(m) ), '' );
            
            if ~exist( nn_base, 'dir' ), error( '%s: dir not found\n', nn_base ); end;
            
            for n=1:length( cnf.st_info )
                weights_name = fullfile( nn_base, sprintf( '%s_weight.dat', cnf.st_info(n).id ) );
                if ~exist( weights_name, 'file' ), error( '%s: not found\n', weights_name ); end
                norm_name = fullfile( nn_base, sprintf( '%s_norm.dat', cnf.st_info(n).id ) );
                if ~exist( norm_name, 'file' ), error( '%s: not found\n', norm_name ); end;
                
                
                %-- Reading weights....
                weights = load( weights_name );
                
                nr_inp = weights(1,1);
                nodes  = weights(2,1);
                
                % -- re-construct network
                net               = feedforwardnet;
                net.numInputs     = 1;
                net.numLayers     = 2;
                net.biasConnect   = [   1;   1 ];
                net.inputConnect  = [   1;   0 ];
                net.layerConnect  = [ 0 0; 1 0 ];
                net.outputConnect = [ 0 1 ];
                
                net.inputs{1}.size = nr_inp; % nr input parameters for input layer
                net.layers{1}.size = nodes;  % hidden layers
                net.layers{1}.transferFcn = 'tansig'; % transfer function for hidden layer
                net.layers{2}.size = 1;      % output layer
                net.layers{2}.transferFcn = 'purelin'; % transfer function for output layer
                
                net.IW{1,1} = weights(3:2 + nodes, 1:nr_inp);
                net.LW{2,1} = weights(3 + nodes, 1:nodes);
                net.b{1}    = weights(4 + nodes,1:nodes)';
                net.b{2}    = weights(5 + nodes,1)';                             
                
                %-- Reading norm...
                norm = load( norm_name);
                
                mean_p = norm(2,:)';
                std_p = norm(3,:)';
                
                mean_t = norm(4,1);
                std_t = norm(5,1);
                
                input_PS = struct( 'xmean', mean_p, 'xstd', std_p, 'ymean', 0, 'ystd', 1 );
                target_PS = struct( 'xmean', mean_t, 'xstd', std_t, 'ymean', 0, 'ystd', 1 );
                
                %-- save network object in matlab file
                fname = fullfile( out_base, sprintf( 'ffnet_%s_%s_day%d.xml',  cnf.st_info(n).id, pol_name{k}, fc_hors(m) ) );
                opaq_writeffnet( fname, net, input_PS, target_PS );
                fprintf( 'Saved %s\n', fname );
            end
        end
    end
end


