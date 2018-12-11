% Correct SO2 statistics
% 
% I noticed during the 
%
% Bino Maiheu

path  = 'param\v3.6\so2\stat_param';
flist = { 'avg_so2_all_agg_time-1h.mat', 'avg_so2_week_agg_time-1h.mat',...
  'avg_so2_weekend_agg_time-1h.mat', 'std_so2_all_agg_time-1h.mat', ...
  'std_so2_week_agg_time-1h.mat', 'std_so2_weekend_agg_time-1h.mat' };

for i = 1:6
  fname = fullfile( path, flist{i});
  s = load( fname );
  if isfield( s, 'xx_avg' )
    xx_avg = s.xx_avg;
    xx_avg( isnan(xx_avg(:,2)) ,2 ) = .5*( xx_avg( isnan(xx_avg(:,2)) ,3 ) + xx_avg( isnan(xx_avg(:,2)) ,25 ) ) ;     
    save( fname, 'xx_avg' );
  else
    xx_std = s.xx_std;
    xx_std( isnan(xx_std(:,2)) ,2 ) = .5*( xx_std( isnan(xx_std(:,2)) ,3 ) + xx_std( isnan(xx_std(:,2)) ,25 ) ) ;     
    save( fname, 'xx_std' );
  end
  
  
end
