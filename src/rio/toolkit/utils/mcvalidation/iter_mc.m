% Monte Carlo
%
% Bino Maiheu


clear all;
close all;


%% Configuration
N     = 100; % number of elements in array
m     = 1;  % draw m elements at the same time
n_min = 100;  % untill each element is drawn n_min times...

% setup Monte Carlo to determine pdf of number of iterations (draws)...
nMC = 10;
iterations = zeros(nMC,1);


%% Here we go
vals = 1:N;
vals = vals(:);

for k=1:nMC;

    if ~mod(k,1000) 
        fprintf( 'MC iter: %d\n', k );
    end
    
    n_valids = zeros(N,1);
    validation_iter     = 1;
    validation_finished = false;
    while ~validation_finished
        % perform random permutation of the indices of available stations
        randIdx = randperm(N);
        st_idx  = randIdx(1:m);
    
        n_valids( st_idx ) = n_valids( st_idx ) + 1;
        if ~any( n_valids < n_min )
            validation_finished = true;
        end
        
        validation_iter = validation_iter + 1;
    end
   
    iterations(k) = validation_iter;        
end


fprintf( 'expectation value = %f (theory for 1 draw: %f)\n', mean( iterations ), N.*sum(1./(1:N)) );
fprintf( 'rel spread        = %f\n', (max(iterations)-min(iterations))/mean(iterations) );


%%
figure( 'Position', [ 100 100 800 300 ] );
%subplot( 2, 1, 1 );
[ n, x ] = hist( iterations, 49);
bar( x, n );
xlabel( 'n iterations needed' );
title( sprintf( 'N=%d, m=%d, n\\_min=%d\n', N, m, n_min ) );
grid on;

%% NOTE THIS IS ONLY THE LAST ITERATION...
%subplot( 2, 1, 2 );
figure( 'Position', [ 100 100 800 300 ]  );
bar(n_valids);
xlim( [ 0.5 N+0.5 ] );
xlabel( 'station number' );
ylabel( '# draws' );

fprintf( 'mean n_valids : %f, std  : %f\n', mean(n_valids), std(n_valids) );

