%RIO_UPDATEPARS
% Update trend parameters, spatial correlations and long term statistics
% in order to prepare for interpolation of the requested date or temporal
% aggregation ( week, weekend, etc... )
%  
%  [ cnf, at ] = rio_updatepars( cnf, date )
%  [ cnf ]     = rio_updatepars( cnf, weekpart, at_hr )
%
% The second optional output argument returns the location in the data
% array, i.e. the column. 
%
% See also rio_init
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ cnf, at ] = rio_updatepars( cnf, varargin )

if nargin == 2
    date = varargin{1};
    %-- Determine date and hour, if relevant...
    vec_date = datevec(date);
    % date_day = datenum(vec_date(1:3));
    date_hour = vec_date(4);
elseif nargin == 3
    weekpart  = varargin{1};
    date_hour = varargin{2} - 1;
else
    error( 'rio_updatepars needs 2 or 3 arguments' );
end

if cnf.agg_time <= 3
    at = 1;
elseif cnf.agg_time == 4
    at = date_hour + 1;
end


%% Load spatial correlations
switch cnf.ipol_mode
    case 'OrdKrig'
        mod_str = 'no_detr';
    otherwise
        mod_str = cnf.gis_type;        
end

%-- Long range corr...
imp_file = fullfile( cnf.paramPath, 'spatial_corr',...
    sprintf( 'p_long_%s_%s_agg_time-%s.mat', cnf.pol_xx, mod_str, cnf.at_lb ) );
%-- Short range corr...
imp_short_file = fullfile( cnf.paramPath, 'spatial_corr', ...
    sprintf( 'p_short_%s_%s_agg_time-%s.mat', cnf.pol_xx, mod_str, cnf.at_lb ) );
if exist( imp_file, 'file' ) && exist( imp_short_file, 'file' )
    p_tmp = load(imp_file);
    cnf.p_corr = p_tmp.p(at,:);
    p_short_tmp = load(imp_short_file);
    cnf.p_corr_short = p_short_tmp.p_short(at,:);
    % We have loaded spatial correlations
    cnf.have_spcorr = true;    
else
    fprintf( '*** rio_updatepars error: unable to locate spatial correlation files\n' );
    cnf.have_spcorr = false;
end




%% Load trend functions for the requested timeslot

 %-- Distinguish between week and weekend, if user did not specify
 %   explicitly
if nargin ~= 3   
    if ~cnf.select_weekpart
        weekpart = 'all';
    else
        % determine whether the requested data is week or weekend
        if ( weekday( date ) == 1 || weekday( date ) == 7 )
            weekpart = 'weekend';
        else
            weekpart = 'week';
        end
    end
end

%-- Trend parameters for mean...
if strcmp( cnf.ipol_mode, 'RIO' )
	imp_file = fullfile( cnf.paramPath, 'trend', ...
			sprintf( 'avg_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, cnf.gis_type, weekpart, cnf.at_lb ) );
	p = load(imp_file);
	cnf.p_avg = p.p_avg(at,:);

	%-- Trend parameters for std dev...
	imp_file = fullfile( cnf.paramPath, 'trend', ...
			sprintf( 'std_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, cnf.gis_type, weekpart, cnf.at_lb ) );
	p = load(imp_file);
	cnf.p_std = p.p_std(at,:);

	%-- Trend parameters for mean error...
	imp_file = fullfile( cnf.paramPath, 'trend', ...
			sprintf( 'avg_err_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, cnf.gis_type, weekpart, cnf.at_lb ) );
	p = load(imp_file);
	cnf.p_avg_err = p.p_avg_err(at,:);

	%-- Trend parameters for std error...
	imp_file = fullfile( cnf.paramPath, 'trend', ...
			sprintf( 'std_err_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, cnf.gis_type, weekpart, cnf.at_lb ) );
	p = load(imp_file);
	cnf.p_std_err = p.p_std_err(at,:);

	% We have trend functions...
	cnf.have_trend = true;

end

%% Update the long term statistics
avg_import_file = fullfile( cnf.paramPath, 'stat_param', ...
    sprintf( 'avg_%s_%s_agg_time-%s.mat', cnf.pol_xx, weekpart, cnf.at_lb ) );
tmp_avg = load(avg_import_file);
cnf.xx_avg = tmp_avg.xx_avg(:,[1 at+1]);

std_import_file = fullfile( cnf.paramPath, 'stat_param', ...
sprintf( 'std_%s_%s_agg_time-%s.mat', cnf.pol_xx, weekpart, cnf.at_lb ) );
tmp_std = load(std_import_file);
cnf.xx_std = tmp_std.xx_std(:,[1 at+1]);

% We have statistics...
cnf.have_stats = true;

end