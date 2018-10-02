% Adjust trend function
% Script to adjust the trendfunction to a predefined trend
%
% In our case : 
% p_ref = the trend for a reference case (Jinan)

clear variables;
close all;

%% Configuration
pol      = 'pm10';
agg_time = 'da';  % or m1, m8 or 1h
wk       = 'all'; % or week or weekend
ref_base = 'D:\Projects\N78H9\Deployments\Jinan\rio-jinan_installed-v1.0';
new_base = 'D:\Projects\N78H9\Deployments\Yangzhou\rio2014';

save_trend = false;

%% Load the reference case
ref_trend = struct();
p = load( fullfile( ref_base, 'rio_param', 'trend', pol, sprintf( 'avg_trend_%s_P1_%s_agg_time-%s.mat', pol, wk, agg_time ) ) );     ref_trend.p_avg = p.p_avg;
p = load( fullfile( ref_base, 'rio_param', 'trend', pol, sprintf( 'avg_err_trend_%s_P1_%s_agg_time-%s.mat', pol, wk, agg_time ) ) ); ref_trend.p_avg_err = p.p_avg_err;
p = load( fullfile( ref_base, 'rio_param', 'trend', pol, sprintf( 'std_trend_%s_P1_%s_agg_time-%s.mat', pol, wk, agg_time ) ) );     ref_trend.p_std = p.p_std;
p = load( fullfile( ref_base, 'rio_param', 'trend', pol, sprintf( 'std_err_trend_%s_P1_%s_agg_time-%s.mat', pol, wk, agg_time ) ) ); ref_trend.p_std_err = p.p_std_err;
clear p;

%% Load the long term averages for the new istallation
p = load( fullfile( new_base, 'param', 'yemc', pol, 'stat_param', sprintf( 'avg_%s_%s_agg_time-%s.mat', pol, wk, agg_time ) ) ); xx_avg = p.xx_avg(:,2);
p = load( fullfile( new_base, 'param', 'yemc', pol, 'stat_param', sprintf( 'std_%s_%s_agg_time-%s.mat', pol, wk, agg_time ) ) ); xx_std = p.xx_std(:,2);
clear p;

% import stations
st_list  = importdata( fullfile( new_base, 'stations', 'yemc', pol, sprintf( '%s_stations_info_GIS_P2.txt', pol ) ) );
beta     = st_list.data(:,4);
st_indic = 0:0.01:2.0;

%% Plot old trend with new stations
% for avg trend
figure;
subplot( 1,2,1) ;
plot( beta, xx_avg, 'ko' );
hold on;
plot( st_indic, polyval( ref_trend.p_avg, st_indic ), 'r-' );
plot( st_indic, polyval( ref_trend.p_avg, st_indic )+polyval( ref_trend.p_avg_err, st_indic ), 'g:' );
plot( st_indic, polyval( ref_trend.p_avg, st_indic )-polyval( ref_trend.p_avg_err, st_indic ), 'g:' );
hold off;
xlabel( '\beta' );
ylabel( 'Average [\mug/m^3]' );
legend( 'new values', 'ref avg trend', 'error' );


subplot( 1,2,2) ;
plot( beta, xx_std, 'ko' );
hold on;
plot( st_indic, polyval( ref_trend.p_std, st_indic ), 'r-' );
plot( st_indic, polyval( ref_trend.p_std, st_indic )+polyval( ref_trend.p_std_err, st_indic ), 'g:' );
plot( st_indic, polyval( ref_trend.p_std, st_indic )-polyval( ref_trend.p_std_err, st_indic ), 'g:' );
hold off;
legend( 'new values', 'ref std trend', 'error' );
xlabel( '\beta' );
ylabel( 'Stddev [\mug/m^3]' );

%% Rescaled trend calculation
med_beta = median(beta);

med_avg      = median(xx_avg);
med_avg_err  = iqr(xx_avg)./1.349; % estimate based on interquartile range, is 1.349 sigma for gaussian
med_std      = median(xx_std);
med_std_err  = iqr(xx_std)./1.349;  

ref_avg  = polyval( ref_trend.p_avg, median(beta) );
ref_avg_err  = polyval( ref_trend.p_avg_err, median(beta) );
ref_std  = polyval( ref_trend.p_std, median(beta) );
ref_std_err  = polyval( ref_trend.p_std_err, median(beta) );

fCorr_avg     = med_avg ./ ref_avg;
fCorr_avg_err = med_avg_err ./ ref_avg_err;
fCorr_std     = med_std ./ ref_std;
fCorr_std_err = med_std_err ./ ref_std_err;

fprintf( 'Median beta value        :: %f\n', med_beta );
fprintf( 'Old reference at beta=%f :: %f\n', med_beta, ref_avg );
fprintf( 'New concentration        :: %f\n', med_avg );
fprintf( 'Correction factor        :: %f\n', fCorr_avg );

p_avg     = fCorr_avg .* ref_trend.p_avg;
p_avg_err = fCorr_avg_err .* ref_trend.p_avg_err;

p_std     = fCorr_std .* ref_trend.p_std;
p_std_err = fCorr_std_err .* ref_trend.p_std_err;

figure;
subplot( 1,2,1) ;
plot( beta, xx_avg, 'ko' );
hold on;
plot( st_indic, polyval( p_avg, st_indic ), 'r-' );
plot( st_indic, polyval( p_avg, st_indic )+polyval( p_avg_err, st_indic ), 'g:' );
plot( st_indic, polyval( p_avg, st_indic )-polyval( p_avg_err, st_indic ), 'g:' );
hold off;
xlabel( '\beta' );
ylabel( 'Average [\mug/m^3]' );
legend( 'new values', 'new avg trend', 'error' );

subplot( 1,2,2) ;
plot( beta, xx_std, 'ko' );
hold on;
plot( st_indic, polyval( p_std, st_indic ), 'r-' );
plot( st_indic, polyval( p_std, st_indic )+polyval( p_std_err, st_indic ), 'g:' );
plot( st_indic, polyval( p_std, st_indic )-polyval( p_std_err, st_indic ), 'g:' );
hold off;
legend( 'new values', 'new std trend', 'error' );
xlabel( '\beta' );
ylabel( 'Stddev [\mug/m^3]' );

%% Writing new trend parameter files with the rescaled trends...
if save_trend
    save( sprintf( 'avg_trend_%s_P2_%s_agg_time-%s.mat', pol, wk, agg_time ), 'p_avg');
    save( sprintf( 'avg_err_trend_%s_P2_%s_agg_time-%s.mat', pol, wk, agg_time ), 'p_avg_err');
    save( sprintf( 'std_trend_%s_P2_%s_agg_time-%s.mat', pol, wk, agg_time ), 'p_std');
    save( sprintf( 'std_err_trend_%s_P2_%s_agg_time-%s.mat', pol, wk, agg_time ), 'p_std_err');
end