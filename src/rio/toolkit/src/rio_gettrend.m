%RIO_GETTREND
% Routine to determine the trend functions. Parameters in the rio_param/trend 
% folder are updated depending on the options specified
% 
% rio_gettrend( cnf )
% rio_gettrend( cnf, opt )
%
% For this function we encode the different options into a structure, of
% which the fields need not necessarily be set :
%
% Standard trend determination options :
%  opt.calcstats ..... : recompute the statistics, default no.
%  opt.time_window ... : use this timewindow when recomputing stats
%  opt.weekpart ...... : either 'all', 'week', 'weekend'
%  opt.overwrite ..... : overwrite the files : 'yes' / 'no' / 'ask' 
%  opt.order_avg ..... : use this polynomial order to fit, default is 2
%                        if a fcn is given below, it is used instead.
%  opt.order_std ..... : use this polynomial order to fit, default is 1
%                        if a fcn is given below, it is used instead.
%  opt.show_stats .... : display some summary statistics on the
%                        regressions
%
% Plotting options : 
%  opt.make_plot ..... : guess...
%  opt.show_plot ..... : display the plot to the screen (def: true)
%  opt.save_plot ..... : saves the plot to emf outputfile (def: false)
%  opt.plot_labels ... : plot station labels as well (def: false)
%  opt.label_dx/_dy .. : shift for labels in x and y
%  opt.label_fontsize  : fontsize for labels (def. 7)
%  opt.fancy_plot .... : different colours for different station types
%  opt.fancy_legend .. : plot fancy_plot legend
%  opt.show_stats .... : show some regression statistics
%
% Specialist options : 
%  opt.fitmode ........... : 'nlinfit' (default) or 'polyfit', if nlinfit, then we
%                           need the statistics toolbox, if polyfit then
%                           not, but then we can only fit polynomials and
%                           in a non-robust way, nlinfit is configured to
%                           use robust fitting here.
%  opt.indic_step ........ : step for horizontal axis in fits (def. 0.001)
%  opt.edgefactor ........ : factor for horizontal axis edge in fits ( def. 10%)
%  opt.order_avg_err . : use this polynomial order to fit
%  opt.order_std_err . : use this polynomial order to fit, default is 1
%  opt.fcn_avg ....... : use this function to fit the avg trend
%  opt.fcn_avg_err ... : use this function to fit the avg trend error
%  opt.fcn_std ....... : use this function to fit the std trend
%  opt.fcn_std_err ... : use this function to fit the std trend error
%  opt.np_avg ........ : number of parameters for avg trend function
%  opt.np_avg_err .... : number of parameters for avg trend err function
%  opt.np_std ........ : number of parameters for std trend function
%  opt.np_std_err .... : number of parameters for std trend err function
%
% See also rio_init, rio_plottrend, rio_calcspcorr, rio_calcstats
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_gettrend( cnf, varargin )

% Some defaults...
if nargin == 1
    error( 'DEFAULT MODE NOT IMPLEMENTED YET...' ); 
    
elseif nargin == 2
    opt = varargin{1};
    if ~isfield( opt, 'calcstats' ), opt.calcstats = false; end;
    if ~isfield( opt, 'weekpart' ), opt.weekpart   = 'all'; end;
    if ~isfield( opt, 'overwrite' ), opt.overwrite = 'no'; end;    
    if ~isfield( opt, 'order_avg' ), opt.order_avg = 2; end;    
    if ~isfield( opt, 'order_std' ), opt.order_std = 1; end;    
    
    [ opt.fcn_avg opt.fcn_avg_err opt.np_avg opt.np_avg_err opt.order_avg_err ] = rio_trendfcn( opt.order_avg );
    [ opt.fcn_std opt.fcn_std_err opt.np_std opt.np_std_err opt.order_std_err ] = rio_trendfcn( opt.order_std );
    
    % Note: BM. 2013-10-22 : I have simpified a little, we can only use 
    % 1 or second degree polynomials for trend fitting, so 
    
    %if ~isfield( opt, 'order_avg_err' ), opt.order_avg_err = 4; end;
    %if ~isfield( opt, 'order_std_err' ), opt.order_std_err = 2; end;
    %if ~isfield( opt, 'fcn_avg' ) || ~isfield( opt, 'np_avg' )        
        % Set standard rio trend functions of the specified order                      
    %elseif ~isfield( opt, 'fcn_avg_err') || ~isfield( opt, 'np_avg_err' )
    %    fprintf( 'rio_gettrend:: need to specify an error parametrisation when providing fcn_avg !\n' );
    %    return;
    %end
    %if ~isfield( opt, 'fcn_std' ) || ~isfield( opt, 'np_std' )
    %    % Set standard rio trend functions of the specified order
    %    
    %elseif ~isfield( opt, 'fcn_std_err') || ~isfield( opt, 'np_std_err' )
    %    fprintf( 'rio_gettrend:: need to specify an error parametrisation when providing fcn_std !\n' );
    %    return;
    %end
    
    
    if ~isfield( opt, 'edgefactor' ), opt.edgefactor = 0.1;       end;
    if ~isfield( opt, 'indic_step' ), opt.indic_step = 0.001;     end;
    if ~isfield( opt, 'make_plot' ), opt.make_plot   = true;      end;
    if ~isfield( opt, 'show_plot' ), opt.show_plot   = true;      end;    
    if ~isfield( opt, 'save_plot' ), opt.save_plot   = false;     end; 
    if ~isfield( opt, 'plot_labels' ), opt.plot_labels = false;   end; 
    if ~isfield( opt, 'label_dx' ), opt.label_dx  = 0.01;    end;
    if ~isfield( opt, 'label_dy' ), opt.label_dy  = 0.50;    end;
    if ~isfield( opt, 'label_fontsize' ), opt.label_fontsize = 7;    end;
    if ~isfield( opt, 'fancy_plot' ), opt.fancy_plot = true; end;
    if ~isfield( opt, 'fancy_legend' ), opt.fancy_legend = true; end;
    if ~isfield( opt, 'fitmode' ), opt.fitmode = 'nlinfit'; end;
    if ~isfield( opt, 'show_stats' ), opt.show_stats = false; end;
else
    error( 'rio_gettrend:: invalid arguments...' );
end

% Loading/generating statistiscs
xx_avg = [];
xx_std = [];
if opt.calcstats   
    % Calculate statistics, relevant options fields are the same,
    % make sure we don't overwrite here...
    stat_opt = opt;
    stat_opt.overwrite = 'no';
    [ xx_avg, xx_std ] = rio_calcstats( cnf, stat_opt );    
else
    % Or load them...
    avg_fname = fullfile( cnf.paramPath, 'stat_param', ...
        sprintf( 'avg_%s_%s_agg_time-%s.mat', cnf.pol_xx, opt.weekpart, cnf.at_lb ) );           
    if ~exist( avg_fname, 'file' )
        error( '%s does not exist', avg_fname );
    end
    tmp = load( avg_fname, 'xx_avg' );    
    xx_avg = tmp.xx_avg;
    
    std_fname = fullfile( cnf.paramPath, 'stat_param', ...
        sprintf( 'std_%s_%s_agg_time-%s.mat', cnf.pol_xx, opt.weekpart, cnf.at_lb ) );  
    if ~exist( std_fname, 'file' )
        error( '%s does not exist', std_fname );
    end
    tmp = load( std_fname, 'xx_std' );
    xx_std = tmp.xx_std;
end


% Fit the trends, 
% only use these station indices, can be integrated in the cnf later on
% use all of them for now
st_list = 1:cnf.nr_st;
% Ignore some stations from the trendfitting...
% this can be programmed a bit nicer ( also the code above is a bit
% redundant.... )
if isfield( opt, 'ignore_stations' )
    if ~isempty( opt.ignore_stations )
        for k=1:length( opt.ignore_stations )
            fprintf( ' - ignoring station %d\n', opt.ignore_stations(k) );
            st_list( st_list == opt.ignore_stations(k) ) = [];
        end        
    end
end

st_idx  = [];
for i=1:length( st_list )
    id = find( cnf.st_info(:,1) == st_list(i) );
    if isscalar( id ),
        st_idx = [ st_idx ; id ];
    end
end

% Set the indicator for making plots and fitting the error
delta = max( cnf.st_indic ) - min( cnf.st_indic );
edge  = opt.edgefactor * delta;
tmp_indic = min( cnf.st_indic )-edge:opt.indic_step:max(cnf.st_indic)+edge;

% fitting mode...
if strcmp( opt.fitmode, 'polyfit' )
    if cnf.agg_time < 4
        ii = find(~isnan(xx_avg(st_idx,2)));
        
        xxx = cnf.st_indic(st_idx);
        yy_avg = xx_avg(st_idx,2);
        yy_std = xx_std(st_idx,2);
            
        % Use simple polyfit, use orders to determine the fitfunctions...
        [ p_avg, S_avg ] = polyfit( xxx(ii), yy_avg(ii), opt.order_avg );
        [ p_std, S_std ] = polyfit( xxx(ii), yy_std(ii), opt.order_std );
    
        [ ~, delta_avg ] = polyval(p_avg, tmp_indic, S_avg );
        p_avg_err = polyfit( tmp_indic, delta_avg, opt.order_avg_err );
    
        [ ~, delta_std ] = polyval(p_std, tmp_indic, S_std );
        p_std_err = polyfit( tmp_indic, delta_std, opt.order_std_err );
    else
        % we have 24 values
        for j = 1:24
            %-- Search for non NaN's
            ii = find(~isnan(xx_avg(st_idx,j+1)));
            
            xxx = cnf.st_indic(st_idx);
            yy_avg = xx_avg(st_idx,j+1);
            yy_std = xx_std(st_idx,j+1);
                        
            [p_avg(j,:), S_avg(j)] = polyfit(xxx(ii), yy_avg(ii), opt.order_avg );           
            [~, delta_avg]     = polyval( p_avg(j,:), tmp_indic, S_avg(j) );
            p_avg_err(j,:)     = polyfit(tmp_indic, delta_avg, opt.order_avg_err );
        
            [p_std(j,:), S_std(j)] = polyfit(xxx(ii), yy_std(ii), opt.order_std );
            [~, delta_std]     = polyval( p_std(j,:), tmp_indic, S_std(j) );
            p_std_err(j,:)     = polyfit(tmp_indic, delta_std, opt.order_std_err );            
            
%             %-- Search for non NaN's
%             % i = find(~isnan(xx_avg(st_idx,j)));
%             
%             [p_avg(j,:), S_avg(j)] = polyfit(cnf.st_indic(st_idx), xx_avg(st_idx,j+1), opt.order_avg );           
%             [dummy, delta_avg]     = polyval( p_avg(j,:), tmp_indic, S_avg(j) );
%             p_avg_err(j,:)         = polyfit(tmp_indic, delta_avg, opt.order_avg_err );
%         
%             [p_std(j,:), S_std(j)] = polyfit(cnf.st_indic(st_idx), xx_std(st_idx,j+1), opt.order_std );
%             [dummy, delta_std]     = polyval( p_std(j,:), tmp_indic, S_std(j) );
%             p_std_err(j,:)         = polyfit(tmp_indic, delta_std, opt.order_std_err );            
%             
            % DEBUG:
            % figure( 'name', sprintf( '%d', j ) ); 
            % plot( cnf.st_indic(st_idx), xx_avg(st_idx,j+1), 'r.'); 
            % hold on; 
            % plot( 0:0.01:0.3, polyval( p_avg(j,:), 0:0.01:0.3 ) );
        end
    end
    
else    
    % use nlinfit in robust mode
    % Confidence interval 100.*(1-alpha) for nlpredci : 68.3 % for 1 sigma !
    ci       = 0.317;
    fit_opts = statset( 'nlinfit' );
    fit_opts = statset(fit_opts, 'Robust', 'on' ); % enable robust fitting
    
    if cnf.agg_time < 4
        ii = find(~isnan(xx_avg(st_idx,2)));
        
        xxx = cnf.st_indic(st_idx);
        yy_avg = xx_avg(st_idx,2);
        yy_std = xx_std(st_idx,2);
            
        % Use simple polyfit, use orders to determine the fitfunctions...
        [ p_avg, S_avg ] = polyfit( xxx(ii), yy_avg(ii), opt.order_avg );
        [ p_std, S_std ] = polyfit( xxx(ii), yy_std(ii), opt.order_std );
        
        % Fit the average trend and it's error...
        [ p_avg, resi_avg, jac_avg, covm_avg, mse_avg ] = ...
            nlinfit( xxx(ii), yy_avg(ii), opt.fcn_avg, zeros(1,opt.np_avg), fit_opts );
        [ dummy, delta_avg ] = nlpredci( opt.fcn_avg, tmp_indic, p_avg, resi_avg, 'covar', covm_avg, 'mse', mse_avg, 'alpha', ci, 'predopt', 'observation' );
        [ p_avg_err ]        = nlinfit( tmp_indic', delta_avg, opt.fcn_avg_err , zeros(1,opt.np_avg_err), fit_opts );
        
        % Fit the std trend and it's error...
        [ p_std, resi_std, jac_std, covm_std, mse_std ] = ...
            nlinfit( xxx(ii), yy_std(ii), opt.fcn_std, zeros(1,opt.np_std), fit_opts );
        [ dummy, delta_std ] = nlpredci( opt.fcn_std, tmp_indic, p_std, resi_std, 'covar', covm_std, 'mse', mse_std, 'alpha', ci, 'predopt', 'observation' );
        [ p_std_err ]        = nlinfit( tmp_indic', delta_std, opt.fcn_std_err, zeros(1,opt.np_std_err), fit_opts );
    else        
        % we have 24 values
        for j = 1:24
            ii = find(~isnan(xx_avg(st_idx,j+1)));
            
            xxx = cnf.st_indic(st_idx);
            yy_avg = xx_avg(st_idx,j+1);
            yy_std = xx_std(st_idx,j+1);
                        
            % Fit the average trend and it's error...
            [ p_avg(j,:), resi_avg, jac_avg, covm_avg, mse_avg ] = ...
                nlinfit( xxx(ii), yy_avg(ii), opt.fcn_avg, zeros(1,opt.np_avg), fit_opts );
            [ dummy, delta_avg ] = nlpredci( opt.fcn_avg, tmp_indic, p_avg(j,:), resi_avg, 'covar', covm_avg, 'mse', mse_avg, 'alpha', ci, 'predopt', 'observation' );
            [ p_avg_err(j,:) ]   = nlinfit( tmp_indic', delta_avg, opt.fcn_avg_err , zeros(1,opt.np_avg_err), fit_opts );
        
            % Fit the std trend and it's error...
            [ p_std(j,:), resi_std, jac_std, covm_std, mse_std ] = ...
                nlinfit( xxx(ii), yy_std(ii), opt.fcn_std, zeros(1,opt.np_std), fit_opts );
            [ dummy, delta_std ] = nlpredci( opt.fcn_std, tmp_indic, p_std(j,:), resi_std, 'covar', covm_std, 'mse', mse_std, 'alpha', ci, 'predopt', 'observation' );
            [ p_std_err(j,:) ]   = nlinfit( tmp_indic', delta_std, opt.fcn_std_err, zeros(1,opt.np_std_err), fit_opts );
        end
    end
end

%  Show the trend with the current options but feed the derived parameters
%  to the function to plot, otherwise the rio_showtrend will read them 
%  from disk.
if opt.make_plot
    rio_plottrend( cnf, opt, xx_avg, xx_std, p_avg, p_avg_err, p_std, p_std_err )
end

% -- Show some regression statistics
if opt.show_stats && cnf.agg_time < 4    
    if strcmp( opt.fitmode, 'polyfit' )
        tr_avg = polyval( p_avg, cnf.st_indic );
        tr_std = polyval( p_std, cnf.st_indic );
    else 
        tr_avg = opt.fcn_avg( p_avg, cnf.st_indic );
        tr_std = opt.fcn_std( p_std, cnf.st_indic );
    end
    fprintf( 'Regression statistics avg trend : \n' );
    disp_stats( tr_avg( st_idx ), xx_avg( st_idx, 2 ) );    
    fprintf( 'Regression statistics std trend : \n' );
    disp_stats( tr_std( st_idx ), xx_std( st_idx, 2 ) ); 
end


% -- Write out the parameters
if strcmpi( opt.overwrite, 'ask' )
    opt.overwrite = lower( questdlg( 'Overwrite the RIO trend parameters ?', ...
        'Overwrite parameters ?', 'Yes', 'No', 'No' ) );    
end
if strcmpi( opt.overwrite, 'yes' )
    % does the pol_xx folder exist ?
    if ~exist( fullfile( cnf.paramPath, 'trend', '' ), 'dir' )
        rio_log( sprintf( 'creating trend parameter folder for %s\n', cnf.pol_xx ) );
        mkdir( cnf.paramPath, 'trend' );
    end
    
    % timestamp for backups
    tstamp = datestr( now, 30 );
    
    % log this !!
    fprintf( 'rio_gettrend: overwriting trend parameters for %s %s agg_time: %s\n', ...
        cnf.pol_xx, opt.weekpart, cnf.at_lb );
    rio_log( sprintf( 'rio_gettrend: overwriting trend for %s %s agg_time: %s', ...
        cnf.pol_xx, opt.weekpart, cnf.at_lb ) );    
       
    % ---------------------------------------------------------------------
    % p_avg
    % ---------------------------------------------------------------------
    trend_file = fullfile( cnf.paramPath, 'trend', ...
        sprintf( 'avg_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, cnf.gis_type, opt.weekpart, cnf.at_lb ) );
    if exist( trend_file, 'file' )
        bak_file = sprintf( '%s.%s.bak', trend_file, tstamp );
        movefile( trend_file, bak_file );
        rio_log( sprintf( ' - backup %s to %s', trend_file, bak_file  ) );
    end
    rio_log( sprintf( ' - save %s', trend_file ) );
    save( trend_file, 'p_avg');
    
    % ---------------------------------------------------------------------
    % p_std
    % ---------------------------------------------------------------------
    trend_file = fullfile( cnf.paramPath, 'trend', ...
        sprintf( 'std_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, cnf.gis_type, opt.weekpart, cnf.at_lb ) );
    if exist( trend_file, 'file' )
        bak_file   = sprintf( '%s.%s.bak', trend_file, tstamp );
        movefile( trend_file, bak_file );
        rio_log( sprintf( ' - backup %s to %s', trend_file, bak_file  ) );
    end
    rio_log( sprintf( ' - save %s', trend_file ) );    
    save( trend_file, 'p_std');
    
    % ---------------------------------------------------------------------
    % p_avg_err
    % ---------------------------------------------------------------------
    trend_file = fullfile( cnf.paramPath, 'trend', ...
        sprintf( 'avg_err_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, cnf.gis_type, opt.weekpart, cnf.at_lb ) );
    if exist( trend_file, 'file' )
        bak_file   = sprintf( '%s.%s.bak', trend_file, tstamp );
        movefile( trend_file, bak_file );
        rio_log( sprintf( ' - backup %s to %s', trend_file, bak_file  ) );
    end
    rio_log( sprintf( ' - save %s', trend_file ) );
    save( trend_file, 'p_avg_err');
    
    % ---------------------------------------------------------------------
    % p_std
    % ---------------------------------------------------------------------
    trend_file = fullfile( cnf.paramPath, 'trend', ...
        sprintf( 'std_err_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, cnf.gis_type, opt.weekpart, cnf.at_lb ) );
    if exist( trend_file, 'file' )
        bak_file   = sprintf( '%s.%s.bak', trend_file, tstamp );
        movefile( trend_file, bak_file );
        rio_log( sprintf( ' - backup %s to %s', trend_file, bak_file  ) );
    end
    rio_log( sprintf( ' - save %s', trend_file ) );
    save( trend_file, 'p_std_err');
end


function disp_stats( x, y )
i = find( ~( isnan(x) | isnan(y) ) );
x = x(i);
y = y(i);

C    = cov( x, y );
R    = C(1,2)/sqrt( C(1,1)*C(2,2) );
fprintf( 'R ........ : %f\n', R );
fprintf( 'R2 ....... : %f\n', R.^2 );
fprintf( 'RICO ..... : %f\n', C(1,2)/C(1,1) );
fprintf( 'RMSE ..... : %f\n', norm( x - y )/sqrt( size(x,1) - 1 ) );
fprintf( 'BIAS ..... : %f\n', mean( x - y ) );


