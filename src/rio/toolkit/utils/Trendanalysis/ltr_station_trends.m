% plot station trends
close all
clear variables

addpath 'D:\Matlab\MatlabToolkit\Mapping\m_map';
addpath 'D:\Matlab\MatlabToolkit\Statistics\MannKendall';

%year = 2000:2011;
year = 2000:2011;
pol  = 'pm10';
gr_n = 73;

avg  = zeros( gr_n, numel(year) );

trend  = zeros( gr_n, 1 );
pval   = zeros( gr_n, 1 );
    
w_trend  = zeros( gr_n, 1 );
w_pval   = zeros( gr_n, 1 );

% load data
for i=1:length(year)
    fname = sprintf( './output/lts/%s/rio_%s_1h_clc06d_4x4_%04d0101-%04d1231.h5', ...
        pol, pol, year(i), year(i) );    
    fprintf( 'Reading %s\n', fname );
    
   avg(:,i) = hdf5read( fname, '/Stations/PostProc/YearAvg' );
end


for k=1:gr_n
    if numel( find(avg(k,:) < 0 ) ) > 10
        continue;
    end
    figure;
    plot( year, avg(k,:), 'ro' );
    xlim( [ 2000 2011] );
    ylim( [ 0 65 ] );
    
    y = avg(k, avg(k,:) > 0 );
    x = year( avg(k,:) > 0 ) ;
    p = polyfit( x, y, 1 );
    hold on;
    plot( 2000:2011, polyval( p, 2000:2011 ), 'k:' );
    hold off;
    fprintf( 'slope = %f µg/m3/year\n', p(1) );
end
        
        
    


