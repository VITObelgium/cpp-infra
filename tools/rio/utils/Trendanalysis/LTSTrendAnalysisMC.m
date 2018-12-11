% LTSTrendAnalysisMC
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
nmc  = 1000;       % number of samplings in MC trend analysis
save_file = true;
mc_mode = 'linreg';

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
st_mk_slope  = -999*ones( n_st, 1 );
st_mk_pval   = -999*ones( n_st, 1 );


%% Retrieving stations trend
fprintf( 'Retrieving stations trend...\n' ); 
for k=1:n_st
    if numel( find(st_average(k,:) < 0 ) ) <= 10
        y = st_avg(k, st_average(k,:) > 0 );
        x = year( st_average(k,:) > 0 ) ;
        
        % mann-kendall test
        [taub tau h sig Z S sigma sen n senplot CIlower CIupper D Dall C3] = ktaub( [ x' y' ], .05, 0 );
        st_mk_slope(k) = sen;
        st_mk_pval(k)  = sig;
    end
    
end

%% Retrieving grid trend
fprintf( 'Retrieving RIO trend...\n' );
if save_file
    fid = fopen( sprintf( 'lts_trend_mcana_%s_%s_rio-%04d-%04d.txt', pol, var, year(1), year(end) ), 'wt' );
    fprintf( fid, 'ID\tMC_MEAN\tMC_STD\tMC_P\n' );
end

mc_store = false;
if mc_store
    rio_mc_slope  = -999*ones( n_gr, nmc );
    rio_mc_pval   = -999*ones( n_gr, nmc );
else
    nmc = 5000;
    rio_mc_slope  = -999*ones( 1, nmc );
    rio_mc_pval   = -999*ones( 1, nmc );
    
    mc_p = ones( n_gr, 1 );
    mc_m = ones( n_gr, 1 );
    mc_s = ones( n_gr, 1 );
end

for k=1:n_gr
    fprintf( 'Randomizing RIO pixel %d\n', k );
    for l=1:nmc
        % add some noize
        mc_avg = rio_avg(k,:)' + rio_err(k,:)'.*randn(size(rio_err(k,:)'));
        
        switch mc_mode
            case 'linreg'
                A = [ ones(length(year),1) year' ];
                b = mc_avg;
    
                [b,bint,r,rint,stats] = regress(b,A);
                if mc_store
                    rio_mc_slope(k,l) =  b(2);
                    rio_mc_pval(k,l)  = stats(3);
                else
                    rio_mc_slope(l) =  b(2);
                    rio_mc_pval(l)  = stats(3);
                end
                    
                    
            case 'mk'
                % mann-kendall test
                datain = [ year' mc_avg ];
                [taub tau h sig Z S sigma sen n senplot CIlower CIupper D Dall C3] = ktaub( datain, .05, 0 );
            
                if mc_store
                    rio_mc_slope(k,l) = sen;
                    rio_mc_pval(k,l)  = sig;
                else
                    rio_mc_slope(l) = sen;
                    rio_mc_pval(l)  = sig;                    
                end
            otherwise 
                error( 'foo bar blah' );
        end
        
        if ~mc_store
            mc_m(k) = mean(rio_mc_slope);
            mc_s(k) = std(rio_mc_slope);
            
            
            % determine how significant the trend is from 0
            % p-value
            if mc_m(k) < 0
                mc_p(k) = normcdf( mc_m(k)/mc_s(k), 0 , 1 );
            else
                mc_p(k) = normcdf(-mc_m(k)/mc_s(k), 0 , 1 );
            end
        end
    end
    
    
    % output
    if save_file
        fprintf( fid, '%d\t%f\t%f\t%f\n', k, mc_m(k), mc_s(k), mc_p(k) );
    end
end
if save_file, fclose(fid); end;

fclose all;

%% pixel analysis
% ip = 1;
% figure;
% errorbar( year, rio_avg(ip,:), rio_err(ip,:), 'ko' );
% hold on;
% for l=1:5        
%     mc_avg = rio_avg(ip,:)' + rio_err(ip,:)'.*randn(size(rio_err(ip,:)'));
%     A = [ ones(length(year),1) year' ];
%     b = mc_avg;
%     
%     [b,bint,r,rint,stats] = regress(b,A);
%     plot( year, polyval( [ b(2) b(1) ], year ), 'r:' );                
% end
% hold off;
% xlabel( 'Year' );
% ylabel( 'Yearly PM_{10} average [µg/m3]' );
% 
% figure( 'Position', [ 100 100 800 300 ] );
% subplot( 1, 2, 1 );
% hist( rio_mc_slope(ip,:), 40 );
% xlabel( 'PM_{10} Trend [µg/m3/yr]' );
% ylabel( 'A.U.' );
% subplot( 1, 2, 2 );
% qqplot( rio_mc_slope(ip,:) );

 

% original->compare
% orig = importdata( 'lts_trend_analysis_pm10_YearAvg_rio-2000-2011.txt' );
% df   = orig.data(:,2)-mean(rio_mc_slope,2);

