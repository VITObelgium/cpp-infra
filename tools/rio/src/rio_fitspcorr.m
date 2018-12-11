%RIO_FITSPCORR
% This routine determines the parameters for the spatial correlation model
% by fitting the correlations, c, versus distances, r.
%
% [ p, p_short, [model] ] = rio_fitspcorr( r, c )
% [ p, p_short, [model] ] = rio_fitspcorr( r, c, opt )
%
% Arguments:
% r .................... : vector with distances in km
% c .................... : vector with correlation values
%
% The routine returns the long range (p) and short range (p_short)
% correlation parameters. The routine fits a spatial correlation model 
% according to some relevant options, set in an optional structure :
%
%  opt.profile ...... : creates a profile histogram first before fitting
%                       and performs a weighted fit from the profile.
%                       Default is false
%  opt.prof_binsize . : binsize for profile, in km, default 20 km
%  opt.corr_model ... : 'exp', 'expm', exponential or modified
%                       exponential, default : exp
%  opt.short ........ : enable short range correlation fitting, default: no
%  opt.short_range .. : the range for short range correlation, default 20
%                       km
%
% See also rio_calcspcorr
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ p, p_short, varargout ] = rio_fitspcorr( r, c, varargin )

if nargin > 2
    opt = varargin{1};
else
    opt = struct();
end
if ~isfield( opt, 'profile' ),      opt.profile      = false; end;
if ~isfield( opt, 'prof_binsize' ), opt.prof_binsize = 20.; end; % km
if ~isfield( opt, 'short' ),        opt.short        = false; end;
if ~isfield( opt, 'short_range' ),  opt.short_range  = 20.; end;
if ~isfield( opt, 'corr_model' ),   opt.corr_model   = 'exp'; end;

% -- get rid of the NaN
i_nan = find( isnan( c ) );
c( i_nan ) = [];
r( i_nan ) = [];

% -- Fit long range trend
if opt.profile
    % -- creating profile for fitting
    r_bounds   = 0.:opt.prof_binsize:max(r)+opt.prof_binsize;
    r_prof     = zeros( 1, length( r_bounds ) - 1 );
    c_prof     = zeros( 1, length( r_bounds ) - 1 );
    c_prof_err = zeros( 1, length( r_bounds ) - 1 );
    
    for k=2:length( r_bounds )
        r_prof(k-1)     = r_bounds(k-1) + .5*( r_bounds(k) - r_bounds(k-1) );
        idx             = find( ( r >= r_bounds(k-1) ) & ( r < r_bounds(k) ) );
        c_prof(k-1)     = mean( c( idx ) );
        c_prof_err(k-1) = std( c( idx ) );
    end

    % -- get rid of NaN ( maybe we didn't have data in a bin...)
    i_nan = find( isnan( c_prof ) );
    r_prof( i_nan ) = [];
    c_prof( i_nan ) = [];
    c_prof_err( i_nan ) = [];        
    
    % DEBUG :
    % r_tmp = 0.:1.:max(r)+opt.prof_binsize;
    % figure;
    % plot( r, c, '.' );
    % hold on;
    % errorbar( r_prof, c_prof, c_prof_err, 'sr' );
    % plot( r_tmp, exp( polyval( p, r_tmp ) ), 'k' ); 
    
    % what do we fit, set it
    r_fit = r_prof';
    c_fit = c_prof';
else
    r_fit = r;
    c_fit = c;
end

% -- Fit long range trend, depends on the model
switch opt.corr_model
    case 'exp' % exponential ( operational version )
        model   = @(p,r)( exp( p(1).*r + p(2)) );
        p_ini   = [ 1. 1. ];
    case 'expm' % modified exponential, more spherical !
        % modified exponential correlation model
        % p(1) = amplitude, p(2) = corr length.
        model  = @(p,r)( p(1).*(1.+r./p(2)).*exp(-r./p(2)) );
        p_ini  = [1. 100. ];
    otherwise
        error( 'rio_fitspcorr: this spatial correlation model is not implemented' );
end
% -- Just fit everything with nlinfit, this way we remain generic for
%    future different models
fit_opts = statset( 'nlinfit' );
fit_opts = statset( fit_opts, 'Robust', 'on' ); % enable robust fitting
% restrict to 500 km for spatial correlations
% TODO: 
[ p, resi, jac, covm, mse ] = nlinfit( r_fit( r_fit < 500), c_fit( r_fit < 500), model, p_ini, fit_opts );
% [ p, resi, jac, covm, mse ] = nlinfit( r_fit, c_fit, model, p_ini, fit_opts );
% fprintf( 'rio_fitspcorr: model %s, mae=%f, mse=%f\n', opt.corr_model, mean(resi), mse );

% some feedback
switch opt.corr_model
    case 'exp'
        fprintf( 'rio_fitspcorr: rho_{long} = %.4f * exp( -r / %.2f )\n', exp(p(2)), -1./p(1) );
    case 'expm'
        fprintf( 'rio_fitspcorr: rho_{long} = %.4f * ( 1 + r / %.2f ) * exp( -r / %.2f )\n', p(1), p(2) );
end

% does the user request the model as well ?
if nargout == 3
    varargout{1} = model;
end

% DEBUG plot
% r_tmp = 0.:1.:max(r)+opt.prof_binsize;
% figure;
% plot( r, c, '.' );
% hold on; 
% plot( r_tmp, model( p, r_tmp ), 'k' );


% -- Fit short range trend, linear robust fit !
if opt.short
    i_short = find( ( r < opt.short_range ) & ( r > 0.010 ) );
    pr      = robustfit( r( i_short ), c( i_short ) );
    p_short = [ pr(2) pr(1) ] ; % change order with robust fit
    
    fprintf( 'rio_fitspcorr:rho_short = %.6f * r + %.4f\n', p_short(1), p_short(2) );
else
    p_short = [ 0 0 ];
end

