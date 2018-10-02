clear variables;
close all;

addpath 'D:\Matlab\MatlabToolkit\Graphs';


st_new = importdata( 'stations/lts/pm10/pm10_stations_info_GIS_clc06d.txt' );

val_new = load( 'validation_da_2009_ltslog_pm10' );
val_old = load( 'validation_da_2009_lts_pm10' );

st_list = st_new.textdata( 2:end, 2);
%avg_new = val_new.valid.bias;
%avg_old = val_old.valid.bias;

%avg_new = val_new.valid.r2;
%avg_old = val_old.valid.r2;

avg_new = val_new.valid.rmse;
avg_old = val_old.valid.rmse;

figure( 'Position', [ 50 50 800 600 ] );
subplot( 3,1,1 );
bar( [ avg_old(1:22); avg_new(1:22) ]' );
ylabel('RMSE [\mug/m^3]');
%ylabel('BIAS [\mug/m^3]');
%ylabel('R^{2}');
set(gca, 'XLim', [0.5 22+0.5 ] );
set(gca, 'XTick', 1:22 );
set(gca, 'XTickLabel', char(st_list{1:22}), 'FontSize', 7 );
rotateticklabel( gca );

subplot( 3,1,2 );
bar( [ avg_old(23:44); avg_new(23:44) ]' );
%ylabel('R^{2}');
%ylabel('BIAS [\mug/m^3]');
ylabel('RMSE [\mug/m^3]');

set(gca, 'XLim', [0.5 22+0.5 ] );
set(gca, 'XTick', 1:22 );
set(gca, 'XTickLabel', char(st_list{23:44}), 'FontSize', 7 );
rotateticklabel( gca );

subplot( 3,1,3 );
bar( [ avg_old(45:61); avg_new(45:61) ]' );
%ylabel('R^{2}');
ylabel('RMSE [\mug/m^3]');
%ylabel('BIAS [\mug/m^3]');
set(gca, 'XLim', [0.5 21+0.5 ] );
set(gca, 'XTick', 1:17 );
set(gca, 'XTickLabel', char(st_list{45:61}), 'FontSize', 7 );
rotateticklabel( gca );

legend( 'lts', 'ltslog');

