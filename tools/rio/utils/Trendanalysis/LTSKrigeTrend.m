% LTSKrigeTrend
%
% Project: N7862 RIO 2012
% Author : Bino Maiheu, (c) bino.maiheu@vito.be 

clear variables;
close all;

set(0, 'DefaultAxesFontName', 'Calibri' );

addpath 'D:\Matlab\MatlabToolkit\Statistics';

pol  = 'o3';
var  = 'YearAvg';
%var  = 'NOE_da';
isel = 3; % reg slop = 3, sen slope = 5

%% load trend data
a = importdata( sprintf( 'lts_trend_analysis_%s_%s_stations-2000-2011.txt', pol, var ) );

st_x     = a.data(:,1)*1e-3;
st_y     = a.data(:,2)*1e-3;
st_slope = a.data(:,isel);

clear a;

st_x( st_slope == -999 ) = [];
st_y( st_slope == -999 ) = [];
st_slope( st_slope == -999 ) = [];
N = length(st_slope);

scatter( st_x, st_y, 25, st_slope, 'filled' );
xlabel( 'X [m]' );
ylabel( 'Y [m]' );

%% compute distance matrix & manual variogram, include zero distance :-)
v    = [ [1:N ; 1:N ]' ; combnk( 1:N,2) ];
r_ij = sqrt((st_x(v(:,1))-st_x(v(:,2))).^2 + (st_y(v(:,1))-st_y(v(:,2))).^2); % convert to km
t_ij = ( st_slope(v(:,1)) - st_slope(v(:,2)) ).^2;

hedges = [ 0. 5. 10. 15. 20. 25. 30. 35. 40. 45. 50. ];
h      = hedges(1:end-1)+.5*diff(hedges);
vg     = zeros(size(h));
vn     = zeros(size(h));
for k=2:length(hedges)
    vn(k-1) = numel( t_ij( r_ij < hedges(k) & r_ij >= hedges(k-1) ) );
    vg(k-1) = 0.5 * mean( t_ij( r_ij < hedges(k) & r_ij >= hedges(k-1) ) );
end

% fvar     = @(x) 1./(2*numel(x)) * sum(x);
% S.val      = accumarray(ixedge,t_ij,[numel(edges) 1],fvar,nan);     
% S.num      = accumarray(ixedge,ones(size(lam)),[numel(edges) 1],@sum,nan);

figure;
subplot( 2, 1, 1 )
d = variogram([st_x st_y], st_slope,'plotit',true,'nrbins',15, 'maxdist',50);
hold on;
plot( h, vg, 'or' ); 
plot( 1:1:50, semivariance( 22.744, 0.0561, 0.0333, 1:1:50, 'spherical' ), 'r:' );
subplot( 2, 1, 2 )
bar( h, vn );


%% determine variogram
figure;
d = variogram([st_x st_y], st_slope, 'plotit', true,'nrbins',15, 'maxdist',50);
a0 = 10; % range
c0 = 0.20; % sill
n0 = 0.03; % nugget

figure;
[a,c,n,S] = variogramfit( d.distance, d.val, a0, c0, [],...
    'solver','fminsearchbnd', 'nugget', c0, 'plotit',true, 'model', 'spherical');
hold on;
plot( 1:1:50, semivariance( 22.744, 0.0561, 0.0333, 1:1:50, 'spherical' ), 'g:' );

%% kriging
disp( '-- Building covariance matrix..' );
C = zeros(N);
for i=1:N
    for j=i:N
        C(i,j) = semivariance( a, c, n, sqrt( (st_x(i)-st_x(j)).^2 + (st_y(i)-st_y(j)).^2 ) , 'spherical' );                
        C(j,i) = C(i,j);
    end
end

% adding space for lambda
C(:,N+1)=1;
C(N+1,:)=1;
C(N+1,N+1)=0;

% Store the inverse
disp( '-- Storing inverse of covariance matrix..' );
Cinv = inv(C);

%% Run over grid and interpolate the residuals
disp( '-- Importing grid...' );
tmp = importdata( 'D:\Projects\N7862\2012\Devel_2012\drivers\lts\pm10\clc06d_grid_4x4.txt' );
gr_x     = tmp.data(:,2)*1e-3;
gr_y     = tmp.data(:,3)*1e-3;
gr_n     = size(tmp.data,1);
gr_slope = size(tmp.data,1);
D        = zeros( N+1, 1 );
clear tmp;

for k=1:gr_n
    fprintf( 'Kriging %f %%\n', 100.*k./gr_n );
    D(1:N) = semivariance( a, c, n, sqrt( ( st_x(:) - gr_x(k) ).^2 + ( st_y(:) - gr_y(k) ).^2 ) , 'spherical' );
    D(N+1) = 1.;
    
    w = Cinv * D;
    
    gr_slope(k) = st_slope' * w(1:N);
end

figure;
rio_map(gr_slope);

%% dump output
fid = fopen( sprintf( 'lts_krige_trend_%s_%s.txt', pol, var ), 'wt' );
fprintf( fid, 'ID\t%s_KRG\n', upper(pol) );
fprintf( fid, '%d\t%f\n', [ 1:gr_n ; gr_slope ]  );
fclose(fid);

fclose all;


