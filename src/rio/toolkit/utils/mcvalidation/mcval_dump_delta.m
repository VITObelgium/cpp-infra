addpath 'D:\Matlab\MatlabToolkit\Validation';

clear all;
close all;

%% Configuration
pol_name = 'pm10';
n_min    = 50;


%% Load the regular leaving one out results
fname = fullfile( 'output', 'v3.6', pol_name, ...
    sprintf( 'rioval_%s_da_clc06d_20130101-20131231.h5', pol_name ) );
    
info          = h5info( fname, '/Validation' );
missing_value = h5readatt( fname, '/', 'MissingValue');
list = [];
list = [ list, strrep( {info.Groups.Name}, '/Validation/', '' ) ];
list = list(:);

rmse = nan(size(list));
bias = nan(size(list));
crcf = nan(size(list));

year = h5read( fname, '/Time/Year' );
mon  = h5read( fname, '/Time/Month' );
day  = h5read( fname, '/Time/Day' );
hour = h5read( fname, '/Time/Hour' );


% compute RMSE for these stations
for i=1:length(list)
           
    mod = h5read( fname, sprintf( '/Validation/%s/mod', list{i}  ) );
    obs = h5read( fname, sprintf( '/Validation/%s/obs', list{i}  ) );
    
    s = validstats( obs, mod, 'missingValue', missing_value );
    
    rmse(i) = s.rmse;
    bias(i) = s.bias;
    crcf(i) = s.r2;
    
    
%     % write output observations    
%     obs(isnan(obs)) = -999;
%     obs(obs<0)      = -999;
%     fid = fopen( sprintf( '%s.csv', list{i} ), 'wt' );
%     fprintf( fid, 'year;month;day;hour;%s;\n', upper(pol_name) );
%     for j=1:length(year)
%         for k=1:24
%             fprintf( fid, '%d;%d;%d;%d;%f;\n',year(j), mon(j), day(j), k, obs(j) );
%         end
%     end
%     fclose(fid);
%     
%     % write model 
%     mod(isnan(mod)) = -999;
%     mod(mod<0)      = -999;
%     fid = fopen( sprintf( 'LEAVE1OUT_%s.csv', list{i} ), 'wt' );
%     fprintf( fid, 'year;month;day;hour;%s;\n', upper(pol_name) );
%     for j=1:length(year)
%         for k=1:24
%             fprintf( fid, '%d;%d;%d;%d;%f;\n',year(j), mon(j), day(j), k, mod(j) );
%         end
%     end
%     fclose(fid);
end

%% we assume  the same station order...


%% Make MC plots for RMSE
pct  = [ 20 ];  % percentage of stations removed in each iteration

for i=1:length(pct)
    p = pct(i);
    
    mc = load( fullfile( 'output', 'v3.6', pol_name, ...
        sprintf( 'riomcval_%s_da_clc06d_20130101-20131231-n_min%d_%dpct.mat', pol_name, n_min, p ) ) );
    
    write_startup( 'startup.ini', mc.xx_val );
    
    % add some other validation statistics
    for j=1:length(mc.xx_val)
        mc.xx_val(j).xx_obs( mc.xx_val(j).xx_obs < 0 )  = NaN;
        mc.xx_val(j).bias = nanmean( mc.xx_val(j).xx_mod-mc.xx_val(j).xx_obs );
        
        mc.xx_val(j).r2 = nan(1,size(mc.xx_val(j).xx_mod,2));
        for k=1:size(mc.xx_val(j).xx_mod,2)
            idx = find( ~isnan( mc.xx_val(j).xx_mod(:,k) ) & ~isnan( mc.xx_val(j).xx_obs(:,k) )  );
            r = corrcoef( mc.xx_val(j).xx_mod(idx,k), mc.xx_val(j).xx_obs(idx,k) );
            mc.xx_val(j).r2(k) = r(1,2).^2;
        end
        
        
        % what validation yields median RMSE & max RMSE
        mm = nanmedian( mc.xx_val(j).rmse );
        mc.xx_val(j).i_med = find( mc.xx_val(j).rmse == mm );
        if isempty( mc.xx_val(j).i_med )
            % get value which is closest to mm, but larger
            df = mc.xx_val(j).rmse - mm;
            df(df<0) = NaN;
            [ ~, mc.xx_val(j).i_med ] = nanmin( df );
        end
        [ ~, mc.xx_val(j).i_max ] = nanmax( mc.xx_val(j).rmse );
    end
    
    for j=1:length(mc.xx_val)
        fid = fopen( sprintf( 'MCMAX_%s.csv', mc.xx_val(j).name ), 'wt' );
        fprintf( fid, 'year;month;day;hour;%s;\n', upper(pol_name) );
        for k=1:length(year)
            for l=1:24
                fprintf( fid, '%d;%d;%d;%d;%f;\n',year(k), mon(k), day(k), ...
                    l, mc.xx_val(j).xx_mod(k,mc.xx_val(j).i_max) );
            end
        end
        fclose(fid);
        
        fid = fopen( sprintf( 'MCMED_%s.csv', mc.xx_val(j).name ), 'wt' );
        fprintf( fid, 'year;month;day;hour;%s;\n', upper(pol_name) );
        for k=1:length(year)
            for l=1:24
                fprintf( fid, '%d;%d;%d;%d;%f;\n',year(k), mon(k), day(k), ...
                    l, mc.xx_val(j).xx_mod(k,mc.xx_val(j).i_med) );
            end
        end
        fclose(fid);
        
    end
 
end




