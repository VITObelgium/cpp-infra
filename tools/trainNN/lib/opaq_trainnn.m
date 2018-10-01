% OPAQ_TRAINNN workhorse training routine
%
%  [ n_samples, r, m, b ] = opaq_trainnn( cnf, station, pol_name, ...
%     agg_str, mor_agg, fc_hor, model_name, start_date, end_date )
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function [ n_samples, r, m, b ] = opaq_trainnn( cnf, ...
    station, pol_name, agg_str, mor_agg, fc_hor, model_name, ...
    start_date, end_date )

% -- create the samples
[ dates, target, input ] = opaq_createsamples( cnf, station, ...
    pol_name, agg_str, mor_agg, fc_hor, model_name, start_date , end_date );
            
n_samples = length(dates);
if isempty(dates)
    error( 'OPAQ:DataNotFound', 'no data found' );    
end

% -- do resampling to be better at predicting episodes
if cnf.ann.resample_uniform,
    [ input, target ] = resample_uniform( input, target );
end
            
fprintf( '%s %s %s day%d - %s : n_samples : %d', pol_name, agg_str, model_name, fc_hor, station, n_samples );
            
% -- apply normalization, note that a transposition is needed here...
[ n_input, input_PS ] = mapstd( input' );
[ n_target, target_PS ] = mapstd( target' );

            
% -- construct network
net = feedforwardnet( cnf.ann.hnodes, 'trainrp' );

net.divideFcn = 'dividerand';
net.divideParam.trainRatio = 0.8;
net.divideParam.valRatio   = 0.2;
net.divideParam.testRatio  = 0.0; % not used during training, we do our own testing --> different year

% Set regularization parameter
% Extract of : http://www.mathworks.nl/support/solutions/en/data/1-17WXC/?solution=1-17WXC
%
% 1) The first method is known as Regularization. This invloves a
%    modification of the performance function which is, by default, the mean
%    sum of squares of the network errors (MSE). Generalization can be
%    improved by modifying this performance function as follows:
%
% MSEREG=g*MSE +(1-g)*MSW
%
% where g is a performance ratio and MSW is the mean sum of sqaures of
% the network weights and biases.
net.performFcn = 'mse';
net.performParam.regularization = cnf.ann.regularization;

% Some other configuration parameters
net.trainParam.show       = 50;  % show result every 50 iterations
net.trainParam.epochs     = 500; % maximum number of iterations
net.trainParam.showWindow = cnf.ann.show_training;

% And finally... train it !
net = train( net, n_input, n_target );
                        
% -- create output folders if they don't exist...
arch_dir = opaq_strrep( cnf.io.archdir_pattern, ...
    'pol', pol_name,'mor_agg', mor_agg, 'fc_hor', fc_hor );
if ~exist( arch_dir, 'dir' ),
    mkdir( arch_dir );
end

fname = opaq_strrep( cnf.io.netfile_pattern, ...
    'station', station, 'pol', pol_name, 'agg_str', agg_str, ...
    'model', model_name, 'mor_agg', mor_agg, 'fc_hor', fc_hor );

opaq_writeffnet( fullfile( arch_dir, fname ), net, input_PS, target_PS );

% now for some output
y = net( n_input );
[r,m,b] = regression( n_target, y );
fprintf( ', result :  r = %f, m = %f, b = %f\n', r, m, b );

if cnf.ann.show_regression
    plotregression(n_target, y );
end


% Reshaping routine : resample or not in order to train ?
function [ input_tr, target_tr ] = resample_uniform( input, target )
        
input_tr  = zeros( size( input ) );
target_tr = zeros( size( target ) );
        
tar_max     = prctile( target, 99 );
tar_min     = min( target );
delta_tar   = tar_max - tar_min;
if delta_tar <= 0
    warning( 'opaq_trainnn:: singularity in target training sample, skipping\n' );    
end
            
for k=1:size(target,1)
    pick = tar_min + rand * delta_tar;
    idx  = rndfind( target, pick ); % see helper routine below
               
    input_tr(k,:) = input(idx,:);
    target_tr(k)  = target(idx);
end
                        
% ------------------------------------------------------------------------
% rndfind helper routine
%
% find index of number in array closest to 'value'
% when more solutions are found, a random one is chosen
 function idx = rndfind( arr, val )

[~,ii] = min( abs( arr-val ) );
alindices = find( arr == arr(ii) );
nri       = max( size( alindices ) );
if (nri > 1)
    nr    = 1 + fix( rand*nri );
    idx   = alindices(nr);
else
    idx   = alindices(1,1);
end            