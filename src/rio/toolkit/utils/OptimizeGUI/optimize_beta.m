function a = optimize_beta( a0, use_a, class_dist, avg, cf )

% Ignore stations : 
tmp_class_dist = class_dist;
tmp_avg        = avg;

% Compute x_idx and x_all, this only feeds the weights we want to optimise
% to the minimizer routine...
x_all = a0;
x_idx = find( use_a == 1 );
a0    = x_all( x_idx );

% Define parameter function to minimize
f = @(x)calc_res(x, x_idx, x_all, tmp_class_dist, tmp_avg, cf );

% Optimization options...
opt = optimset( 'Display', 'iter', 'Simplex', 'on' );

% Minimize w.r.t. to weights
n = length( a0 ) ;

% Try constrained minimisation : positive weights and sum of weights
% smaller than some value... max beta ?
if cf.constrain
    fprintf( 'Starting constrained minimisation...\n' );
    %lb = a0 - 0.01 .* cf.constrainFactor .* a0;
    %lb( lb < 0 ) = 0.;
    %ub = a0 + 0.01 .* cf.constrainFactor .* a0;
    lb = cf.lb;
    ub = cf.ub;
    for i=1:n, fprintf( '%f -> [%f ; %f ]\n', a0(1,i), lb(1,i), ub(1,i) ); end;
    a = fmincon( f, a0, -eye(n), zeros(n,1), [], [], lb, ub, [], opt );
else
    % Constrain only on positive weights
    a = fmincon( f, a0, -eye(n), zeros(n,1), [], [], [], [], [], opt );
end

% reset a
x_all( x_idx ) = a;
a              = x_all;

end

function res = calc_res( x, x_idx, x_all, class_dist, pol_avg, cf )
% build the full array 
x_all(x_idx) = x;

% Calculate the beta with the non fixed a parameters
beta = zeros( length( pol_avg), size(x_all,1) );
for i=1:size(x_all,1)
    beta(:,i) = calc_beta( x_all(i,:), class_dist{i} );
end

fit_opts = statset('nlinfit');
fit_opts = statset(fit_opts, 'Robust', 'on' );

if size(x_all,1)==1
    switch( cf.order )
        case 1
            fcn = @(p,x)p(1)*x + p(2);
            ndf  = 2;
        case 2
            fcn = @(p,x)p(1)*x.^2 + p(2)*x + p(3);
            ndf = 3;
    end
else
    switch( cf.order )
        case 1
            fcn = @(p,x)p(1)*x(:,1) + p(2)*x(:,2)+ p(3);
            ndf = 3;
        case 2
            fcn = @(p,x)p(1)*x(:,1).^2 + p(2)*x(:,2).^2 + p(3)*x(:,1).*x(:,2) + p(4)*x(:,1) + p(5)*x(:,2)+ p(6);            
            ndf = 6;
    end
end
% Do the fit...
[p ,res ] = nlinfit( beta, pol_avg, fcn, ones(1,ndf), fit_opts );

% Now get the MAE / MSE...
switch cf.norm
  case 'TRENDCORR'
    %res = sum( abs( res ) ) ./ length( x_all );
    C = corrcoef( pol_avg, fcn(p,beta) );
    res = 1 - C(1,2).^2;
  case 'TRENDMSE'
    res = sum( res.^2 ) ./ length( x_all );
  case 'CORR'
    C = corrcoef( pol_avg, beta );
    res = 1 - C(1,2).^2;
  otherwise
    error( 'Unknown mode norm mode...' );
end

end
