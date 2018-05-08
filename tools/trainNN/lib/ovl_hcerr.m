% -------------------------------------------------------------------------
% OVL (c) VITO / IRCEL
%
% Developed by Jef Hooybergs, Stijn Janssen, Peter Stouthuysen, Bino Maiheu
% Rewritten by Bino Maiheu (c) VITO 2010
% -------------------------------------------------------------------------
% 
% hc_err = ovl_calchcerr( cnf, fc_hor, xx_date, xx_pol, pm10_fc, hc_range )
% 
% This function computes the hindcast error for the specified range
%

function hc_err = ovl_hcerr( fc_hor, xx_date, xx_pol, fc_pol, hc_range )

hc_err = -999 * ones( size(xx_date,1), hc_range );

%----------------------------------------
%-- Determine the errors in the forecast
%-- over the hindcast periode
%----------------------------------------
day_N = fc_hor;

%-- Errors for 'day N, day N-1,... day 0, day -1 ... day - hindcast'
%-- NOTE: day N = day i in loop
max_err = day_N + 1 + hc_range;  % max nr of entries in the err array
recent_err(1:max_err) = -999; 

%-- Position of day -1
day_m1_pos = day_N + 1 + 1; 

%-- Initialization for i = 1
recent_err(1) = fc_pol(1) - xx_pol(1);
orig_err(1)   = recent_err(1);

%-- Loop over all data
nr_data = length(xx_date);
for i=2:nr_data
    %----------------------
    %-- Forecast procedure
    %----------------------

    date_gap = xx_date(i) - xx_date(i-1);
    if date_gap > 1
        %-- Missing value in data series:
        %-- shift error values back in time
        tmp_NAN(1:min(max_err,date_gap-1)) = -999.;
        recent_err = shift_errors(recent_err, min(max_err,date_gap-1), tmp_NAN );
        clear tmp_NAN;
    end
    
    %-- Update recent errors for day i (= day N)
    %-- new value is original error of day N
    orig_err(i) = fc_pol(i) - xx_pol(i);
    recent_err  = shift_errors(recent_err, 1, orig_err(i));            
    
    %-- Get forecast error
    hc_err(i,:) = recent_err(day_m1_pos:max_err);
end

hc_err( hc_err == -999 ) = NaN;


function recent_err = shift_errors(recent_err, date_gap, orig_err)

%-------------------------------------
%-- Shift all errors "date_gap" days 
%-- to the past and add best_err 
%-- at most recent place
%-------------------------------------

nr_err = length(recent_err);

%-- shift errors
perm = zeros(nr_err);
perm(1:nr_err-date_gap+1, date_gap:nr_err) = diag(ones(nr_err - date_gap,1),1);
recent_err = recent_err * perm;

%-- add most recent
recent_err(1, 1:length(orig_err)) = orig_err;

