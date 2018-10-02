% LTSTrendAnalysis
%
% Project: N7862 RIO 2012
% Author : Bino Maiheu, (c) bino.maiheu@vito.be 

clear variables;
close all;

set(0, 'DefaultAxesFontName', 'Calibri' );

addpath 'D:\Matlab\MatlabToolkit\Mapping\m_map';
addpath 'D:\Matlab\MatlabToolkit\Statistics\MannKendall';

year = 2000:2011; % array with year numbers
pol  = 'no2';    % pollutant
var  = 'YearAvg'; % variable to get trend for
%var  = 'NOE_da'; % variable to get trend for
nmc  = 100;       % number of samplings in MC trend analysis
save_file = false;

%% Collect yearly average and other yearly statistics
fname = sprintf( './output/lts/%s/rio_%s_1h_clc06d_4x4_%04d0101-%04d1231.h5', pol, pol, year(1), year(1) ); 
n_gr  = numel( hdf5read( fname, '/Grid/XCoord') );
n_st  = numel( hdf5read( fname, '/Stations/XCoord') );

rio_fcov = zeros( size( year ) );
rio_avg  = zeros( n_gr, numel(year) );
rio_err  = zeros( n_gr, numel(year) );

st_avg   = zeros( n_st, numel(year) );
st_average   = zeros( n_st, numel(year) );
st_name  = hdf5read( fname, '/Stations/Name' );
st_x     = double( hdf5read( fname, '/Stations/XCoord' ) );
st_y     = double( hdf5read( fname, '/Stations/YCoord' ) );


for i=1:length(year)
   fname = sprintf( './output/lts/%s/rio_%s_1h_clc06d_4x4_%04d0101-%04d1231.h5', pol, pol, year(i), year(i) );    
    fprintf( 'Reading %s\n', fname );
   rio_fcov(i) = h5readatt( fname, '/Grid/PostProc/', 'FCov' ); 
    
   rio_avg(:,i) = hdf5read( fname, sprintf( '/Grid/PostProc/%s', var ) );
   rio_err(:,i) = hdf5read( fname, '/Grid/PostProc/YearAvgErr' );   
   
   st_avg(:,i)  = hdf5read( fname, sprintf( '/Stations/PostProc/%s', var ) );
   
   % this is just to have the average no matter what, used in a test in the regression loop to check 
   % whether station has data, a bit hacky-wacky, but screw it.
   st_average(:,i) = hdf5read( fname, '/Stations/PostProc/YearAvg' );
end

%% Book some arrays, initialise to -999
st_reg_slope = -999*ones( n_st, 1 );
st_reg_pval  = -999*ones( n_st, 1 );
st_mk_slope  = -999*ones( n_st, 1 );
st_mk_pval   = -999*ones( n_st, 1 );


rio_reg_slope = -999*ones( n_gr, 1 );
rio_reg_pval  = -999*ones( n_gr, 1 );
rio_wreg_slope = -999*ones( n_gr, 1 );
rio_wreg_pval  = -999*ones( n_gr, 1 );
rio_mk_slope  = -999*ones( n_gr, 1 );
rio_mk_pval   = -999*ones( n_gr, 1 );

% durbin-watson statistic
rio_dw_pval   = -999*ones( n_gr, 1 );
rio_dw        = -999*ones( n_gr, 1 );

%% Retrieving stations trend
fprintf( 'Retrieving stations trend...\n' ); 
if save_file
    fid = fopen( sprintf( 'lts_trend_analysis_%s_%s_stations-%04d-%04d.txt', pol, var, year(1), year(end) ), 'wt' );
    fprintf( fid, 'STATION\tX\tY\tREG_SLOPE\tREG_P\tMK_SLOPE\tMK_P\n' );
end
for k=1:n_st
    if numel( find(st_average(k,:) < 0 ) ) <= 10
        y = st_avg(k, st_average(k,:) > 0 );
        x = year( st_average(k,:) > 0 ) ;
        
        
        
        
        % linear regression
        [b,bint,r,rint,stats] = regress( y', [ ones(length(x),1) x' ] );        
            
        st_reg_slope(k) = b(2); % in µg/m3/year
        st_reg_pval(k)  = stats(3);
        
        %if strcmp( st_name(k).Data, '41N043' ) || strcmp( st_name(k).Data, '43R240' )
            figure( 'Position', [ 100 100 600 300 ] ) ;
            plot( x, y, 'ok');
            hold on;
            plot( year, polyval( [ b(2) b(1) ], year ), 'k-' );
            line( [ year(1)-.5 year(end )+.5 ], [ 40 40 ], 'Color', 'r', 'LineStyle', ':' );
            hold off;            
            
            xlim( [ year(1)-.5 year(end )+.5 ] );
            ylim( [ 0 60 ] );
            xlabel( 'year' );
            ylabel( 'PM_{10} yearly average [µg/m3]' );
            grid on;
            title( st_name(k).Data );
            text( 2000.2, 50., sprintf( 'Trend = %.2f µg/m^3/yr (p-value: %.3f)\n', st_reg_slope(k), st_reg_pval(k)  ), ...
                'FontName', 'Calibri' );
            saveas( gcf, sprintf( '%s_%s_trend_%s.emf', pol, var, st_name(k).Data ), 'emf' );
            close(gcf);
        %end
        
        
        % p = polyfit( x, y, 1 );
        
        % mann-kendall test
        [taub tau h sig Z S sigma sen n senplot CIlower CIupper D Dall C3] = ktaub( [ x' y' ], .05, 0 );
        st_mk_slope(k) = sen;
        st_mk_pval(k)  = sig;
    end
    
    if save_file
        fprintf( fid, '%s\t%d\t%d\t%f\t%f\t%f\t%f\n', st_name(k).Data, st_x(k), st_y(k), st_reg_slope(k), st_reg_pval(k), ...
            st_mk_slope(k), st_mk_pval(k) );
    end
end
if save_file, fclose( fid ); end;

%% Retrieving grid trend
fprintf( 'Retrieving RIO trend...\n' );
if save_file
    fid = fopen( sprintf( 'lts_trend_analysis_%s_%s_rio-%04d-%04d.txt', pol, var, year(1), year(end) ), 'wt' );
    fprintf( fid, 'ID\tREG_SLOPE\tREG_P\tWREG_SLOPE\tWREG_P\tMK_SLOPE\tMK_P\n' );
end

fid_dw = fopen( sprintf( 'lts_dwpval_%s_%s_rio-%04d-%04d.txt', pol, var, year(1), year(end) ), 'wt' );
fprintf( fid_dw, 'ID\tDW\tDW_P\n' );
for k=1:n_gr
    
    % ordinary regression
    A = [ ones(length(year),1) year' ];
    b = rio_avg(k,:)';
    
    [b,bint,r,rint,stats] = regress(b,A);
    
    % perform Durboin-Watson test for auto-correlation in the regression
    % residuals
    [ rio_dw_pval(k) rio_dw(k) ]  = dwtest(r,A);
    
    rio_reg_slope(k) = b(2); % in µg/m3/year
    rio_reg_pval(k)  = stats(3);
    
%     fg = figure();
%     plot( year, rio_avg(k,:), 'ko' );
%     hold on;
%     plot( year, polyval( [ b(2) b(1) ], year ), 'r-' );
%     ylim( [ 30 60 ] );
%     hold off;
%     drawnow;
%     %waitforbuttonpress
%     close(fg);
    
    
    
    % weighted regression
    [ par, ci, p, h ] = weighted_trend( year, rio_avg(k,:), rio_err(k,:), 0.05 );
    rio_wreg_slope(k) = par(1);
    rio_wreg_pval(k)  = p;
    
    % mann-kendall test
    datain = [ year' rio_avg(k,:)' ];
    [taub tau h sig Z S sigma sen n senplot CIlower CIupper D Dall C3] = ktaub( datain, .05, 0 );
    rio_mk_slope(k) = sen;
    rio_mk_pval(k)  = sig;
                
    % output
    if save_file
        fprintf( fid, '%d\t%f\t%f\t%f\t%f\t%f\t%f\n', k, rio_reg_slope(k), rio_reg_pval(k), rio_wreg_slope(k), rio_wreg_pval(k), ...
            rio_mk_slope(k), rio_mk_pval(k) );
    end
    
    fprintf( fid_dw, '%d\t%f\t%f\n', k, rio_dw(k), rio_dw_pval(k) );
end
if save_file, fclose(fid); end;
fclose(fid_dw);
fclose all;




