% Long Term Trend analysis for RIO
close all
clear variables

set(0, 'DefaultAxesFontName', 'Calibri' );

addpath 'D:\Projects\N7862\2012\Devel_2012\src';
addpath 'D:\Matlab\MatlabToolkit\Mapping\m_map';
addpath 'D:\Matlab\MatlabToolkit\Statistics\MannKendall';

%year = 2000:2011;
year = 2001:2011;
pol  = 'o3';
gr_n = 2098;  % should be able to read this from the RIO file...

fcov = zeros( size( year ) );
avg  = zeros( gr_n, numel(year) );
err  = zeros( gr_n, numel(year) );

trend  = zeros( gr_n, 1 );
pval   = zeros( gr_n, 1 );
    
w_trend  = zeros( gr_n, 1 );
w_pval   = zeros( gr_n, 1 );


n_mc = 1000;
mk_slope  = zeros( gr_n, 1 );
mk_pval   = zeros( gr_n, 1 );
mkorig_pval   = zeros( gr_n, 1 );
mkorig_pval_mc = zeros( gr_n, n_mc );


% load data
for i=1:length(year)
    fname = sprintf( './output/lts/%s/rio_%s_1h_clc06d_4x4_%04d0101-%04d1231.h5', ...
        pol, pol, year(i), year(i) );    
    fprintf( 'Reading %s\n', fname );
   fcov(i) = h5readatt( fname, '/Grid/PostProc/', 'FCov' ); 
    
   avg(:,i) = hdf5read( fname, '/Grid/PostProc/YearAvg' );
   err(:,i) = hdf5read( fname, '/Grid/PostProc/YearAvgErr' );   
end

% figure;
% plot( year, fcov, '.' );
% xlabel( 'Year' );
% ylabel( 'FCov - 1h' );
% title( pol );

%
%figure;
for k=1:2098
    disp(k);
    %errorbar( year, avg(k,:), err(k,:), 'xr' );
    %ylim( [ 10 40 ] );
    %xlim( [ 1999 2012 ] );
   
    
    pp = polyfit( year, avg(k,:), 1 );    
   
    
    A = [ ones(length(year),1) year' ];
    b = avg(k,:)';
    
    [b,bint,r,rint,stats] = regress(b,A);
    %hold on;
    %plot( year, b(1)+b(2).*year, 'k:' );
    
    trend(k) = b(2); % in µg/m3/year
    pval(k)  = stats(3);
        
    
    [ par, ci, p, h ] = weighted_trend( year, avg(k,:), err(k,:), 0.05 );
    w_trend(k) = par(1);
    w_pval(k)  = p;

    
    % Mann-Kendall test for trends
    datain = [ year' avg(k,:)' ];
    [taub tau h sig Z S sigma sen n senplot CIlower CIupper D Dall C3] = ktaub( datain, .05, 0 );
    mk_slope(k) = sen;
    mk_pval(k)  = sig;
    
    fprintf( 'polyfit slope = %f µg/m3/year, regress slop = %f µg/m3/year, mk slope = %f, w slope = %f\n', pp(1), b(2), sen, par(1) );
    
    
    [H,p_value] = Mann_Kendall( avg(k,:), 0.05 );
    mkorig_pval(k) = p_value;
        
    
    %% hmm.... is this sensible ??
    avg_rnd = bsxfun( @plus, avg(k,:)', bsxfun( @times, err(1,:)',  randn(numel(err(1,:)),n_mc) ) );
     
    for l=1:n_mc    
        [H,p_value] = Mann_Kendall( avg_rnd(:,l), 0.05 );
        mkorig_pval_mc(k,l) = p_value;
    end
    
    %drawnow;
end

%%
figure;
rio_map( trend, 'µg/m3/year', sprintf( '%s regression slope', pol ) );
caxis( [ -1.5 1.5 ])

figure;
rio_map( pval, 'p-value', sprintf( '%s regression p-value', pol ) );
caxis( [ 0 0.5 ])

figure;
rio_map( w_trend, 'µg/m3/year', sprintf( '%s regression slope (weighted)', pol ) );
caxis( [ -1.5 1.5 ])

figure;
rio_map( w_pval, 'p-value', sprintf( '%s regression p-value (weighted)', pol ) );
caxis( [ 0 0.5 ])

figure;
rio_map( mk_slope, 'µg/m3/year', sprintf( '%s Mann Kendall Sen Slope', pol ) );
caxis( [ -1.5 1.5 ])

figure;
rio_map( mk_pval, 'p-value', sprintf( '%s Mann Kendall p value', pol ) );
caxis( [ 0 0.5 ])

figure;
rio_map( mkorig_pval, 'p-value', sprintf( '%s Original Mann Kendall p value', pol ) );
caxis( [ 0 0.5 ])
