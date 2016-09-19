%OPAQ_EXTRACT_METEO Load meteo variables from netcdf file...
%
% [ xx_date, xx_meteo ] = opaq_extract_meteo( ncfile, coords )
%
% Where 
%  - ncfile is a netcdf file created by the opaq meteo scripts (FNL or GFS)
%  - coords is a nx2 array of lon, lat gridvalues to extract
%
% Current OPAQ meteo parameters defined here are as follows :
%
%  P1  - 2m temperature
%  P2  - 2m relative humidity
%  P3  - 10m v component of wind
%  P4  - 10m u component of wind
%  P5  - planetary boundary layer heights
%  P6  - Total cloud cover entire atmosphere
%  P7  - high cloud cover
%  P8  - low cloud cover
%  P9  - medium cloud cover
%  P10 - 975mb - 1000mb inversion strength
%  P11 - 950mb - 1000mb inversion strength (~500 m )
%  P12 - 925mb - 1000mb inversion strength 
%  P13 - 900mb - 1000mb inversion strength (~1 km)
%  P14 - 850mb - 1000mb inversion strength (~1.5 km)
%  P15 - dwsp/dz (shear stress) between bottom two layers (975mb - 1000mb)
%
% Author: Bino Maiheu, (c) bino.maiheu@vito.be

function [ xx_date, xx_meteo, varargout ] = opaq_extract_meteo( ncfile, coords )

% -------------------------------------------------------------------------
% -- set some values
% -------------------------------------------------------------------------

gr_n   = size(coords,1);
gr_lon = coords(:,1);
gr_lat = coords(:,2);

i_lon = zeros(size(gr_lon));
i_lat = zeros(size(gr_lat));

% -------------------------------------------------------------------------
% -- reading grid
% -------------------------------------------------------------------------
lat = ncread( ncfile, 'latitude' );  % get latitude array
lon = ncread( ncfile, 'longitude' ); % get longitude array
time= ncread( ncfile, 'time' );      % get times, seconds since 1970/1/1
    
% -- convert times in the file to matlab datetime
xx_date = datenum(1970,1,1,0,0,0)+time/3600/24;
disp(datestr(xx_date));

    
% -------------------------------------------------------------------------
% -- determine the grid cell indices in the lat/lon arrays
% -------------------------------------------------------------------------
for k=1:gr_n,
    ii = find( abs( gr_lon(k)-lon ) < 1e-6 );
    jj = find( abs( gr_lat(k)-lat ) < 1e-6 );
        
    if numel(ii) ~= 1 || numel(jj) ~= 1
        error( 'Cannot identify lat/lon in arrays...' );
    end
        
    i_lon(k) = ii;
    i_lat(k) = jj;
end;
    
clear lon lat time;

% -------------------------------------------------------------------------
% -- read fields
% -------------------------------------------------------------------------
t2m  = ncread( ncfile, 't2m' );
rh2m = ncread( ncfile, 'rh2m' );
u10m = ncread( ncfile, 'u10m' );
v10m = ncread( ncfile, 'v10m' );
    
blh  = ncread( ncfile, 'blh' );
    
cctot  = ncread( ncfile, 'cloud_cover' );
cclow  = ncread( ncfile, 'low_cloud_cover' );
ccmid  = ncread( ncfile, 'middle_cloud_cover' );
cchigh = ncread( ncfile, 'high_cloud_cover' );

t3d    = ncread( ncfile, 't' );
wsp3d  = sqrt( ncread( ncfile, 'u' ).^2 + ncread( ncfile, 'v' ).^2 );
hgt3d  = ncread( ncfile, 'hgtprs' ); % geopotential in gpm


% -------------------------------------------------------------------------    
% -- perform some computations, own derived variables..
% -------------------------------------------------------------------------
    
% compute the inversion strength in °C as a simple temperature
% difference between the lowest model level (1000mb) and the respective
% level
iv975mb = squeeze(t3d(:,:,2,:)-t3d(:,:,1,:)); % 975 - 1000 mb
iv950mb = squeeze(t3d(:,:,3,:)-t3d(:,:,1,:)); % 950 - 1000 mb
iv925mb = squeeze(t3d(:,:,4,:)-t3d(:,:,1,:)); % 925 - 1000 mb
iv900mb = squeeze(t3d(:,:,5,:)-t3d(:,:,1,:)); % 900 - 1000 mb
iv850mb = squeeze(t3d(:,:,6,:)-t3d(:,:,1,:)); % 850 - 1000 mb

% compute the shear stress, without the kinematic viscosity,
% so simply the vertical temperature gradient : difference between
% 1000 mb level and 975 mb level divided by the difference in geopotential
% heigth
tau975mb = (wsp3d(:,:,2,:)-wsp3d(:,:,1,:))./(hgt3d(:,:,2,:)-hgt3d(:,:,1,:));
    
clear t3d wsp3d hgt3d;
        
% -------------------------------------------------------------------------
% -- loop over gridcells requested
% -------------------------------------------------------------------------
xx_meteo = nan(gr_n,15,length(xx_date));
for k=1:gr_n
        
    ii=i_lon(k);
    jj=i_lat(k);
        
    xx_meteo(k,1,:)  = t2m(ii,jj,:);  %P1
    xx_meteo(k,2,:)  = rh2m(ii,jj,:); %P2
    xx_meteo(k,3,:)  = v10m(ii,jj,:); %P3
    xx_meteo(k,4,:)  = u10m(ii,jj,:); %P4
    xx_meteo(k,5,:)  = blh(ii,jj,:);  %P5
        
    xx_meteo(k,6,:)  = cctot(ii,jj,:);  %P6
    xx_meteo(k,7,:)  = cchigh(ii,jj,:); %P7
    xx_meteo(k,8,:)  = cclow(ii,jj,:);  %P8
    xx_meteo(k,9,:)  = ccmid(ii,jj,:);  %P9
        
    xx_meteo(k,10,:) = iv975mb(ii,jj,:); %P10
    xx_meteo(k,11,:) = iv950mb(ii,jj,:); %P11
    xx_meteo(k,12,:) = iv925mb(ii,jj,:); %P12
    xx_meteo(k,13,:) = iv900mb(ii,jj,:); %P13
    xx_meteo(k,14,:) = iv850mb(ii,jj,:); %P14
        
    xx_meteo(k,15,:) = tau975mb(ii,jj,:); %P15
    
end % end loop over the gridcells

% If requested return the parameter names
if nargout > 2
    varargout{1} = opaq_meteo_parnames;
end


function [ list ] = opaq_meteo_parnames

list = [ ...
    struct( 'short', 't2m', 'long', '2m temperature' ); ...
    struct( 'short', 'rh2m', 'long', '2m relative humidity' ); ...
    struct( 'short', 'v10m', 'long', '10m v component of wind' ); ...
    struct( 'short', 'u10m', 'long', '10m u component of wind' ); ...
    struct( 'short', 'blh', 'long', 'planetary boundary layer height' ); ...
    struct( 'short', 'cctot', 'long', 'total cloud cover entire atmosphere' ); ...
    struct( 'short', 'cchigh', 'long', 'high cloud cover' ); ...
    struct( 'short', 'cclow', 'long', 'low cloud cover' ); ...
    struct( 'short', 'ccmid', 'long', 'medium cloud cover' ); ...
    struct( 'short', 'iv975mb', 'long', '975mb - 1000mb inversion strength' ); ...
    struct( 'short', 'iv950mb', 'long', '950mb - 1000mb inversion strength (~500 m )' ); ...
    struct( 'short', 'iv925mb', 'long', '925mb - 1000mb inversion strength' ); ...
    struct( 'short', 'iv900mb', 'long', '900mb - 1000mb inversion strength (~1 km)' ); ...
    struct( 'short', 'iv850mb', 'long', '850mb - 1000mb inversion strength (~1.5 km)' ); ...
    struct( 'short', 'tau975mb', 'long', 'dwsp/dz (shear stress) between bottom two layers (975mb - 1000mb)' ); ...
    ];
