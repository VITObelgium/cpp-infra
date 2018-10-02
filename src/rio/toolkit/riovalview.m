%RIOVALVIEW Process, analyse and visualize the RIO validation files
%
% riovalview( flist, tags )
% riovalview( flist, tags, options )
%
% Where flist is a cell array of the filenames of the hdf5 validation
% output files and tags is a cell array of the associated tags for each
% configuration/setup. 
%
% View the validation for a set of RIO leaving-one-out files with
% associated tag. 
%
% See also validstats, targplot, boxplot, bar
%
% Developed by Bino Maiheu, (c) VITO 2010-2013
% RIO (c) VITO/IRCEL 2004-2013
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function riovalview( flist, tags, varargin )

assert(~any(size(flist)-size(tags)), 'Differing size for filelist and taglist.' );

% default options
opts = struct( 'make_barplots', true, ...
  'make_boxplots', true );
%opts.skip_stations = { '41N043', '41R002', '42R802', '41B006', '41B008' };
opts.skip_stations = { };
%warning( 'skipping a few stations...' );
%waitforbuttonpress;


if nargin > 2
  if ~isstruct( varargin{1} ),
    error( 'Expection option structure...' );
  end
  p = varargin{1};
  if isfield(p, 'make_barplots' ), opts.make_barplots = p.make_barplots; end;
  if isfield(p, 'make_boxplots' ), opts.make_boxplots = p.make_boxplots; end;
  if isfield(p, 'skip_stations' ), opts.skip_stations = p.skip_stations; end;
  
end

set(0, 'DefaultAxesFontName', 'Calibri' );
fprintf( 'Welcome to the RIO Validation viewer...\n' );

% Get all stations in all files...
st_list = riovv_getstations( flist, opts );
n_st    = length( st_list );
n_tags  = length( flist );
fprintf( 'Discovered %d unique stations...\n', n_st );
st_type  = nan(n_st,1);
st_x     = nan(n_st,1);
st_y     = nan(n_st,1);

st_typename = { 'Rural', 'Urban background', 'Urban', 'Industrial', 'Traffic' };

% Temporal validation...
val = cell( n_st, n_tags );

for i=1:n_tags
  % Read information from this tag file
  info          = h5info( flist{i}, '/validation' );
  station       = strrep( {info.Groups.Name}, '/validation/', '' );
  pol_name      = h5readatt( flist{i}, '/', 'Pollutant');  
  agg_timestr   = h5readatt( flist{i}, '/', 'AggregationTime');  
  gis_type      = h5readatt( flist{i}, '/', 'Proxy');  
  missing_value = h5readatt( flist{i}, '/', 'MissingValue');
  
  xx_date = h5read( flist{i}, '/time/matlab' );
  
  % Loop over the stations
  for j=1:length(station)
    
    if ~isempty( find( strcmp( station{j}, opts.skip_stations ) ) )
      fprintf( 'Skipping %s...\n', station{j} );
      continue
    end
    
    mod = h5read( flist{i}, sprintf( '/validation/%s/mod', station{j}  ) );
    obs = h5read( flist{i}, sprintf( '/validation/%s/obs', station{j}  ) );
    
   
    
    % reading station type (overwriting previous info, no check on whether
    % we have differences... ah well...)
    st_type(j) =  h5readatt( flist{i}, sprintf( '/validation/%s', station{j}  ), 'type' );
    st_x(j)    =  h5readatt( flist{i}, sprintf( '/validation/%s', station{j}  ), 'x' );
    st_y(j)    =  h5readatt( flist{i}, sprintf( '/validation/%s', station{j}  ), 'y' );
    
    
    
    [ s, fh ] = validstats( [ xx_date obs ], [ xx_date mod ], ...
      'missingValue', missing_value, ...
      'exceedThresh', 50., ...
      'rpeValue', 100.*(365-35)/365, ...
      'makePlot', false, ...
      'obsLabel', sprintf( '%s [\\mug/m^3]', station{j} ), ...
      'modLabel', sprintf( 'RIO %s [\\mug/m^3]', strrep( tags{i}, '_', '\_' ) ), ...
      'plotTitle', sprintf( '%s %s %s, %s - %s', upper(pol_name), gis_type, agg_timestr, ...
                                                 strrep( tags{i}, '_', '\_' ), station{j} ), ...
      'dateTicks', true, ...
      'targetMax', 1.5 );
    
    fprintf( 'Validating %s (idx=%d, obs avg = %f)\n', station{j}, j, s.avg_obs );
    
    
    % Multiply the RPE with 100 to have a % value
    s.rpe = 100*s.rpe;
    
    % Store the cell array
    val{j,i} = s;
    
    % find the index in the st_list
    %     idx = find( strcmp( st_list, station{j}) );
    %     rmse(idx,i) = s.rmse;
    
    % save plot for this station
    if ishandle(fh)
      saveas( fh, sprintf( 'rioval_%s_%s_%s_%s-%s.emf', pol_name, gis_type, agg_timestr, tags{i}, station{j} ), 'emf' );
      close(fh);
    end
  end
end

%% make some temporal validation plots
colormap summer;
set(0, 'DefaultAxesFontName', 'Calibri' );
set(0, 'DefaultAxesFontSize', 11 );
scrsz = get(0,'ScreenSize');
pos   = [scrsz(3)/8 scrsz(4)/8 6*scrsz(3)/8 4*scrsz(4)/8];
pos2   = [scrsz(3)/8 scrsz(4)/8 6*scrsz(3)/8 4*scrsz(4)/10];
fprintf( 'Producing temporal validation comparison plots...\n' );
if opts.make_barplots
  hFig_rmse  = figure( 'Position', pos );
  riovv_barplot( val, 'rmse', 'units', '\mug/m^3', 'stations', station, 'tags', tags );
  
  hFig_absbias  = figure( 'Position', pos );
  riovv_barplot( val, 'absbias', 'units', '\mug/m^3', 'stations', station, 'tags', tags );
  
  hFig_bias  = figure( 'Position', pos );
  riovv_barplot( val, 'bias', 'units', '\mug/m^3', 'stations', station, 'tags', tags );
  
  hFig_tcor  = figure( 'Position', pos );
  riovv_barplot( val, 'r2', 'units', 'n/a', 'stations', station, 'tags', tags );
end

if opts.make_boxplots
  hFig_box1  = figure( 'Position', pos2 );
  riovv_boxplot( val, { 'rmse', 'absbias', 'target', 'r2' }, ...
    'units', { '\mug/m^3', '\mug/m^3', '', '' }, 'tags', tags );
  
  hFig_box2  = figure( 'Position', pos2 );
  riovv_boxplot( val, { 'si', 'fcf', 'ffa', 'rpe' }, 'units', { '', '', '', '' }, 'tags', tags );
end

%% make some target plots
figure( 'Position', [scrsz(3)/6 scrsz(4)/6 2*scrsz(3)/6 3*scrsz(4)/6] );
tx = nan(size(val));
ty = nan(size(val));
for i=1:numel(val), 
  if isfield( val{i}, 'targ_x' ) && isfield( val{i}, 'targ_y' )
    tx(i) = val{i}.targ_x;
    ty(i) = val{i}.targ_y;
  else
    tx(i) = NaN;
    ty(i) = NaN;
  end
end;
targplot(tx,ty,'inputMode', 'coords', ...
  'catLabel', strrep( tags, '_', '\_' ), 'catSymbol', { 'k.', 'ko', 'r.', 'ro' }, 'targetMax', 1.5 );

%% some spatial validation plots
% get unpaired station averages for model and observations and dump them
% again through the validstats routine...
mod = nan(size(val));
obs = nan(size(val));
for i=1:numel(val), 
  if isfield( val{i}, 'avg_obs' ) && isfield( val{i}, 'avg_mod' )
    mod(i) = val{i}.avg_mod;
    obs(i) = val{i}.avg_obs;
  else
    mod(i) = NaN;
    obs(i) = NaN;
  end
end;


figure;
boxplot( mod-obs );
line( [ 0 size(mod,2)+0.5 ], [ 0 0 ], 'Color', 'k', 'LineStyle', ':' );
ylabel( 'Mod - Obs [\mug/m^3]' );
set(gca, 'XLim', [0.5 size(mod,2)+0.5 ] );
set(gca, 'XTick', 1:size(mod,2) );
set(gca, 'XTickLabel', char(tags), 'FontSize', 11 );
title( 'Spatial validation', 'FontSize', 11 );

figure( 'Position', pos );
n_tags = size(mod,2);
for i_tag=1:n_tags
  ss{i_tag} = validstats( obs(:,i_tag), mod(:,i_tag), 'storeData', true );
  
  subplot( 1, n_tags, i_tag );
  ppmin = 0.9*min( [ ss{i_tag}.o; ss{i_tag}.m ] );
  ppmax = 1.1*max( [ ss{i_tag}.o; ss{i_tag}.m ] );
  dpp   = ppmax-ppmin;
  plot( ss{i_tag}.o, ss{i_tag}.m, '.' );
  xlabel( 'Observation' );
  ylabel( sprintf( 'Model - %s', strrep(tags{i_tag}, '_', '\_') ) );
  grid on;
  xlim( [ ppmin ppmax ] );
  ylim( [ ppmin ppmax ] );
  title( strrep(tags{i_tag}, '_', '\_') );
  hold on;
  line( [ ppmin ppmax ], [ ppmin ppmax ], 'Color', 'r' );
  line( [ ppmin ppmax], [ ppmin ppmax/2 ], 'Color', 'k', 'LineStyle', '--' );
  line( [ ppmin ppmax], [ ppmin 2.*ppmax ], 'Color', 'k', 'LineStyle', '--' );
  plot( ppmin:ppmax, polyval( [ ss{i_tag}.slope ss{i_tag}.icept ], ppmin:ppmax ), 'g-' );
  % add some text
  text( ppmin+0.05*dpp, ppmax-0.05*dpp, { ...
    sprintf( 'RMSE \t= %.2f', ss{i_tag}.rmse ); ...
    sprintf( 'BIAS \t= %.2f', ss{i_tag}.bias ); ...
    sprintf( 'R^2  \t= %.2f', ss{i_tag}.r2 ) }, ...
    'FontName', 'Calibri', 'FontSize', 10, 'BackgroundColor', [ 1. .8 .8 ], ...
    'LineStyle', '-', 'LineWidth', 1, 'EdgeColor', 'k', ...
    'HorizontalAlignment', 'left', 'VerticalAlignment', 'top' );    
end


% Plots of general spatial indicators as bar charts
set(0, 'DefaultAxesFontSize', 10 );
figure;
subplot(2,2,1);
riovv_spbar( ss, tags, 'rmse', 'RMSE [\mug/m^3]', 'Spatial RMSE' );

subplot(2,2,2);
riovv_spbar( ss, tags, 'r2', 'Correlation R^2', 'Spatial correlation', [ 0 1 ] );

subplot(2,2,3);
riovv_spbar( ss, tags, 'bias', 'BIAS [\mug/m^3]', 'Spatial BIAS' );

fprintf('All done.\n');


function riovv_spbar( ss, tags, fld, ylab, tit, varargin )
vito_style;
n_tags = length(ss);
v = size(1,n_tags);
for i=1:n_tags, v(i) = ss{i}.(fld); end;
bar(v, 'FaceColor', vito_orange, 'EdgeColor', vito_black );
ylabel( ylab );
title( tit );
set(gca, 'XLim', [0.5 n_tags+0.5 ] );
set(gca, 'XTick', 1:n_tags );
set(gca, 'XTickLabel', char(tags), 'FontSize', 12 );
if nargin > 5
  ylim( varargin{1} );
end


function riovv_barplot( val, indic, varargin )

p = inputParser;
p.addParamValue( 'units','', @isstr );
p.addParamValue( 'stations', {}, @iscell );
p.addParamValue( 'tags', {}, @iscell );
p.parse( varargin{:} );

if ~isfield( val{1}, indic )
  error( 'riovalview', 'unknown field in validation' );
end

v = nan(size(val));
for i=1:numel(val), 
  if isfield( val{i}, indic ) 
    v(i) = val{i}.(indic);
  else 
    v(i) = NaN;
  end
end;
bar(v);
set(gca,'Unit','normalized','Position',[0.05 0.15 0.9 0.8]);
if ~isempty(p.Results.units)
  ylabel( sprintf( '%s - %s', upper(indic), p.Results.units ) );
else
  ylabel( upper(indic) );
end
set(gca, 'XLim', [0.5 size(v,1)+0.5 ] );
set(gca, 'XTick', 1:size(v,1) );
if ~isempty( p.Results.stations )
  set(gca, 'XTickLabel', char(p.Results.stations), 'FontSize', 11 );
  rotateticklabel(gca,90,8);
end
if ~isempty( p.Results.tags )
  legend( strrep( p.Results.tags, '_', '\_' ), 'FontSize', 11 );
end


function riovv_boxplot( val, indic_list, varargin )
p = inputParser;
p.addParamValue( 'units','', @iscell );
p.addParamValue( 'tags', {}, @iscell );
p.parse( varargin{:} );

n=numel(indic_list);
for i=1:n
  if ~isfield( val{1}, indic_list{i} )
    error( 'riovalview', 'unknown field in validation' );
  end
    
  subplot(1,n,i);
  v = nan(size(val));
  for j=1:numel(val), 
    if isfield( val{j}, indic_list{i} ) 
      v(j) = val{j}.(indic_list{i});
    else
      v(j) = NaN;
    end    
  end;

  boxplot( v );
  if ~isempty( p.Results.units{i} ) 
    ylabel( sprintf( '%s - %s', upper(indic_list{i}), p.Results.units{i} ) );
  else 
    ylabel( upper(indic_list{i}) );
  end
  set(gca, 'XLim', [0.5 size(v,2)+0.5 ] );
  set(gca, 'XTick', 1:size(v,2) );
  if ~isempty( p.Results.tags )
    set(gca, 'XTickLabel', char(p.Results.tags), 'FontSize', 11 );
  end
end




% Reads all the station names from each file and returns a list of unique
% station identifiers
function list = riovv_getstations( flist, opts ) 
list = {};
for k=1:length(flist)
  info = h5info( flist{k}, '/validation' );
  list = [ list, strrep( {info.Groups.Name}, '/validation/', '' ) ];  
end
list = unique( list );
for k=1:length(opts.skip_stations); 
  idx=find( strcmp( opts.skip_stations{k}, list ) ); 
  list(idx) = []; 
  disp(idx); 
end
