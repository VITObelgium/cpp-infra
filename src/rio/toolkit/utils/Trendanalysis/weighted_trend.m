% weighted_trend
%
% Function to compute the linear trend slope parameter taking into account
% the standard errors on each of the data points. First a weighted
% regression is performed
%
% [ par, ci, p, h ] = weighted_trend( t, x, e, varargin )
%
% - inputs : 
%      t : independant variable (e.g. time for timeseries)
%      x : dependant variable
%      e : standard error for dependant variable ( 1 sigma errorbars)
%
% - outputs : 
%      par : regression line parameters (so they can be ingested in
%            polyval), in other words y = ax + b  -> par = [ a b ].
%      ci  : confidence interval at the 1-alpha significance level for the 
%            slope
%      p   : p-value : probability that the observed trend is induced
%            by random fluctuations in the data p = 0.05 means that there
%            is only 5 % chance that the observed trend is induced by 
%            random behaviour of the data. 
%      h   : 1 a trend is observed at the 1-alpha significance level
%            0 no trend is observed ....
%
% Bino Maiheu, (c) VITO 2012

function [ par, ci, p, h ] = weighted_trend( t, x, e, varargin )

tail  = 0.;     % two tailed test
alpha = 0.05;   % significance level
if nargin > 3
    alpha = varargin{1};
    if nargin > 4
        tail = varargin {2};
    end
end

n = length(t);
A = [ ones(n,1) t' ];
b = x';
w = 1./e.^2; % inverse variances as weights for the regression

[r,stdr] = lscov(A,b,w);

%a  = r(1);
b  = r(2);     % slope parameter
sb = stdr(2);  % standard error on b

tb = b/sb;     % t statistic for b

% number of degrees of freedom
df    = n-2;

% Compute the correct p-value for the test, and confidence intervals
if tail == 0 % two-tailed test
    p = 2 * tcdf(-abs(tb), df);
    crit = tinv((1 - alpha / 2), df) .* sb;
    ci = [ b - crit, b + crit ];
elseif tail == 1 % right one-tailed test
    p = tcdf(-tb, df);
    crit = tinv(1 - alpha, df) .* sb;
    ci = [ b - crit, Inf(size(p)) ];
elseif tail == -1 % left one-tailed test
    p = tcdf(tb, df);
    crit = tinv(1 - alpha, df) .* sb;
    ci = [ -Inf(size(p)), b + crit ];
end
% Determine if the actual significance exceeds the desired significance
h = cast(p <= alpha, class(p));

% return regression parameters so they can be used with polyval
par = [ r(2) r(1) ];


