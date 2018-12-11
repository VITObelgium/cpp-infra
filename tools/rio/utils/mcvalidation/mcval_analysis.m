% mcval_analysis
% produce boxplots of the validation statistics...

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
% compute RMSE for these stations
for i=1:length(list)
           
    mod = h5read( fname, sprintf( '/Validation/%s/mod', list{i}  ) );
    obs = h5read( fname, sprintf( '/Validation/%s/obs', list{i}  ) );
    
    s = validstats( obs, mod, 'missingValue', missing_value );
    
    rmse(i) = s.rmse;
    bias(i) = s.bias;
    crcf(i) = s.r2;
end

%% we assume  the same station order...


%% Make MC plots for RMSE
sel_types    = [ 1, 2, 3, 4 ];
str_types    = { 'rural', 'urb.back', 'urban', 'industry' };
pct          = [ 10 20 50 ];  % percentage of stations removed in each iteration

fid = fopen( 'mcval_analysis.txt', 'at' );

for ii=1:numel(sel_types)
    sel_type = sel_types(ii);
    str_type = str_types{ii};
    
    for i=1:length(pct)
        p = pct(i);
        
        mc = load( fullfile( 'output', 'v3.6', pol_name, ...
            sprintf( 'riomcval_%s_da_clc06d_20130101-20131231-n_min%d_%dpct.mat', pol_name, n_min, p ) ) );
        
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
        
        
        % select only stations of specific type
        if sel_type > -1
            idx_type = find( [mc.xx_val.type] == sel_type );
        else
            idx_type = 1:numel(mc.xx_val);
        end
        
        % plot RMSE
        grp = {};
        for j=1:numel(mc.xx_val(idx_type))
            grp = [ grp, repmat( { mc.xx_val(idx_type(j)).name }, 1, numel(mc.xx_val(idx_type(j)).rmse) ) ];
        end
        
        figure( 'Position', [ 100 100 800 400 ] )
        boxplot( [mc.xx_val(idx_type).rmse], grp );
        set( findobj(gca,'Type', 'text'), 'String', '' );
        hold on;
        plot( rmse(idx_type) , 'k*' );
        
        L = length(list(idx_type));
        
        ylim( [ -2 16 ] );
        line( [0.5 L+0.5 ], [ 0 0 ], 'LineStyle', '-', 'Color', 'k' );
        set(gca, 'XLim', [0.5 L+0.5 ] );
        set(gca, 'XTick', 1:L );
        set(gca, 'XTickLabel', char(list(idx_type)), 'FontSize', 10 );
        th = rotateticklabel(gca,270,10);
        grid on;
        legend( 'leaving one out' );
        ylabel( 'RMSE [µg/m3]' );
        
        title( sprintf( 'RMSE %s (%s), n\\_min: %d, pct removed: %d, type: %s', pol_name, str_type, n_min, p ) );
        saveas( gcf, sprintf( 'mcval_%s_%s_rmse_nmin_%d_pct_%d.emf', pol_name, str_type, n_min, p ), 'emf' );
        
        % plot R2
        grp = {};
        for j=1:numel(mc.xx_val(idx_type))
            grp = [ grp, repmat( { mc.xx_val(idx_type(j)).name }, 1, numel(mc.xx_val(idx_type(j)).r2) ) ];
        end
        
        figure( 'Position', [ 100 100 800 400 ] )
        boxplot( [mc.xx_val(idx_type).r2], grp );
        set( findobj(gca,'Type', 'text'), 'String', '' );
        hold on;
        plot( crcf(idx_type) , 'k*' );
        
        ylim( [ 0.4 1.0 ] );
        line( [0.5 L+0.5 ], [ 0 0 ], 'LineStyle', '-', 'Color', 'k' );
        set(gca, 'XLim', [0.5 L+0.5 ] );
        set(gca, 'XTick', 1:L );
        set(gca, 'XTickLabel', char(list(idx_type)), 'FontSize', 10 );
        th = rotateticklabel(gca,270,10);
        grid on;
        legend( 'leaving one out' );
        ylabel( 'R^2' );
        
        title( sprintf( 'R^2 %s (%s), n\\_min: %d, pct removed: %d, type: %s', pol_name, str_type, n_min, p ) );
        saveas( gcf, sprintf( 'mcval_%s_%s_r2_nmin_%d_pct_%d.emf', pol_name, str_type, n_min, p ), 'emf' );
        
        % plot BIAS
        grp = {};
        for j=1:numel(mc.xx_val(idx_type))
            grp = [ grp, repmat( { mc.xx_val(idx_type(j)).name }, 1, numel(mc.xx_val(idx_type(j)).bias) ) ];
        end
        
        figure( 'Position', [ 100 100 800 400 ] )
        boxplot( [mc.xx_val(idx_type).bias], grp );
        set( findobj(gca,'Type', 'text'), 'String', '' );
        hold on;
        plot( bias , 'k*' );
        
        ylim( [ -10. 10. ] );
        line( [0.5 L+0.5 ], [ 0 0 ], 'LineStyle', '-', 'Color', 'k' );
        set(gca, 'XLim', [0.5 L+0.5 ] );
        set(gca, 'XTick', 1:L );
        set(gca, 'XTickLabel', char(list(idx_type)), 'FontSize', 10 );
        th = rotateticklabel(gca,270,10);
        grid on;
        legend( 'leaving one out' );
        ylabel( 'BIAS [µg/m3]' );
        
        title( sprintf( 'BIAS %s, n\\_min: %d, pct removed: %d, type: %s', pol_name, n_min, p, str_type ) );
        saveas( gcf, sprintf( 'mcval_%s_%s_bias_nmin_%d_pct_%d.emf', pol_name, str_type, n_min, p ), 'emf' );
        
        fprintf( 'LEAVE-ONE-OUT :\n' );
        fprintf( ' - median rmse : %f\n', nanmedian( rmse(idx_type) ) );
        fprintf( ' - median bias : %f\n', nanmedian( bias(idx_type) ) );
        fprintf( ' - median r2   : %f\n', nanmedian( crcf(idx_type) ) );
        fprintf( 'MONTE CARLO :\n');
        fprintf( ' - median rmse (all MC results) : %f\n', nanmedian( [mc.xx_val(idx_type).rmse] ) );
        fprintf( ' - median bias  (all MC results) : %f\n', nanmedian( [mc.xx_val(idx_type).bias] ) );
        fprintf( ' - median r2  (all MC results) : %f\n', nanmedian( [mc.xx_val(idx_type).r2] ) );
        
        
        fprintf( fid, '%s\t%d\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\n', pol_name, sel_type, n_min, p, ...
            nanmedian( rmse(idx_type) ), ...
            nanmedian( bias(idx_type) ), ...
            nanmedian( crcf(idx_type) ), ...
            nanmedian( [mc.xx_val(idx_type).rmse] ), ...
            nanmedian( [mc.xx_val(idx_type).bias] ), ...
            nanmedian( [mc.xx_val(idx_type).r2]   ) );                               
                
        % use ... leave one out, median & worst !
        
    end
end


fclose( fid );


