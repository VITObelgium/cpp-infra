%TSCORR_GAUSSIANANOMALY Timeseries correction via Gaussian anomaly detection
%
%   [ xx_date_new, xx_vals_new ] = tscorr_gaussiananomaly( xx_date, xx_vals, opts )
%
% Options structure
%  - opts.start_date       : start date for the training periode
%  - opts.end_date         : end date
%  - transform_idx         : which transform to use, currently ;: 
%                            1 : none
%                            2 : log(1+x)
%                            3 : sqrt(x)
%                            4 : pow(x,p)
%  - transform_p           : transformation parameter
%  - nbins                 : number of bins to take in histogramming
%  - alpha                 : the cutoff value expressed as probability,
%                            default is 0.005, meaning only the 0.5 %
%                            outliers are rejected, note that for 
%                            the lower or upper boundary we each use
%                            alpha/2
%  - make_plot             : plots the histogram and fit
%  - apply_lower           : apply to the lower boundary
%  - apply_upper           : apply to the upper boundary
%
% Bino Maiheu, (c) 2014 bino.maiheu@vito.be

function [ xx_date_new, xx_vals_new ] = tscorr_gaussiananomaly( xx_date, xx_vals, opts )
   
% -- set some default values
if ~isfield( opts, 'start_date' ), opts.start_date = xx_date(1); end;
if ~isfield( opts, 'end_date' ), opts.end_date = xx_date(end); end;
if ~isfield( opts, 'transform_idx' ), opts.transform_idx = 1; end;
if ~isfield( opts, 'transform_p' ), opts.transform_p = 1; end;
if ~isfield( opts, 'nbins'), opts.nbins = 100; end;
if ~isfield( opts, 'alpha' ), opts.alpha = 0.005; end;
if ~isfield( opts, 'make_plot' ), opts.make_plot = false; end;
if ~isfield( opts, 'apply_lower' ), opts.apply_lower = true; end;
if ~isfield( opts, 'apply_upper' ), opts.apply_upper = true; end;

% first apply transform to input 
switch( opts.transform_idx )
    case 1, % no transform
        xx = xx_vals;
        xlab = 'C';
    case 2, % log(1+x)
        xx = log(1+xx_vals);
        xlab = 'log(1+C)';
    case 3, % sqrt(x)
        xx = sqrt(xx_vals);
        xlab = 'sqrt(C)';
    case 4, % pow(x,p)
        xx = xx_vals.^opts.transform_p;
        xlab = sprintf('pow(C,%.3f)', opts.transform_p);
    otherwise
        errordlg( 'Invalid tranform requested', 'Gaussian Anomaly Detection Error', 'modal' );       
end

fprintf( 'Gaussian anomaly detection, training period from : %s to %s\n', ...
    datestr(opts.start_date), datestr(opts.end_date) );

xx = xx( xx_date >= opts.start_date & xx_date <= opts.end_date );
if isempty(xx) || isempty(find(~isnan(xx)))
    errordlg( 'No valid data in selected training period', 'Gaussian Anomaly Detection Error', 'modal' );
    return;
end

% make histogram
[ h, x ] = hist( xx , opts.nbins );

% Normalize the pdf, don't take into account bin width, we're going to use
% a 3 parameter fit and renormalise the resulting cumulative sum
% afterwards
dx = median(diff( x ));
dh = 0.5*(h(1:length(h)-1) + h(2:length(h)));
s = sum( dx .* dh );
h = h ./ s;

% initial guesses for gaussian fit
[dummy,ii] = max(h);

% Define cost function function
% p(1) : mu
% p(2) : sigma
p0 = [ x(ii) abs(fwhm(x,h)) ];
lb = [ ];
ub = [ ];
fprintf( 'Fitting gaussian, mu0=%f, sig0=%f\n', p0(1), p0(2) );
options = optimset( 'fminsearch' );
costfun = @(p,x,h) sum( ( pdf_gauss(x, p(1), p(2) ) - h ).^2 );
fitp    = fminsearchbnd(@(p) costfun(p,x,h),p0,lb,ub,options);

mu    = fitp(1);
sigma = fitp(2);
fprintf( ' --> final parameters, mu=%f, sigma=%f\n', mu, sigma );

g = pdf_gauss( x, mu, sigma );
C = corrcoef(g,h);
fprintf( ' --> RRMSE : %f\n', sqrt( mean( ( g-h).^2 ) )./ std(h) );
fprintf( ' --> R2    : %f\n', C(1,2)*C(2,1) );

if opts.make_plot
    figure;
    stairs(x,h);
    hold on;
    plot( x, g, 'r' );
    grid on;
    
    xr = get( gca, 'XLim' );
    yr = get( gca, 'YLim' );
    dx = xr(2)-xr(1);
    dy = yr(2)-yr(1);
    ylabel('A.U.');
    xlabel( xlab )
    
    text( 0.1, 0.8, { ...
        sprintf( 'Gaussian PDF fit' ); ...
        sprintf( ' - mu    : %f', mu ); ...
        sprintf( ' - sigma : %f', sigma ); ...
        sprintf( 'Statistics' ) ; ...
        sprintf( ' - RRMSE : %.3f', sqrt( mean( ( g-h).^2 ) )./ std(h) ); ...
        sprintf( ' - R2    : %.3f', C(1,2)*C(2,1) ); ...
        } , 'FontName', 'Calibri', 'Units', 'normalized' );
end

% find lower and upper limit from inverse cdf
x_lo = norminv(opts.alpha/2,mu,sigma);
x_hi = norminv(1-opts.alpha/2,mu,sigma);

switch( opts.transform_idx )
    case 1, % no transform
    case 2, % log(1+x)
        x_lo = exp(x_lo)-1;
        x_hi = exp(x_hi)-1;            
    case 3, % sqrt(x)
        x_lo = x_lo.^2;
        x_hi = x_hi.^2;
    case 4, % pow(x,p)
        x_lo = x_lo.^(1./(opts.transform_p));
        x_hi = x_hi.^(1./(opts.transform_p));        
end

% apply
xx_vals_new = xx_vals;
xx_date_new = xx_date;
if opts.apply_lower 
    fprintf( 'Applying cut off for lower limit : %f\n', x_lo );
    xx_vals_new( xx_vals_new < x_lo ) = NaN;    
end
if opts.apply_upper
    fprintf( 'Applying cut off for upper limit : %f\n', x_hi );
    xx_vals_new( xx_vals_new > x_hi ) = NaN;
end
