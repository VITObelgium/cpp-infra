%RIO_PLOTTREND
% Make some plots of the trend functions.
% 
% rio_plottrend( cnf, opt )
% rio_plottrend( cnf, opt, xx_avg, xx_std, p_avg, p_avg_err, p_std, p_std_err )
%
% If they averages and trend paramters are not given explicitly as
% arguments, they are read from the current model configuration, this
% routine can therefore be used to visualise the currently loaded trend
% models depending on the configuration structure.
%
% opt configuration structure, the same options are used as for the 
% rio_gettrend function, but not all options are taken into account. Here 
% is a list of the relevant options for the plotting function :
%  - opt.weekpart ...... : 'all', 'week' or 'weekend'
%  - opt.show_plot ..... : display the plot to the screen (def: true)
%  - opt.save_plot ..... : saves the plot to emf outputfile (def: false)
%  - opt.plot_labels ... : plots the station labels with the points
%  - opt.label_dx ...... : shift from data point for labels in x
%  - opt.label_dy ...... : shift from data point for labels in y
%  - opt.label_fontsize  : fontsize for labels (def. 7)
%  - opt.fancy_plot .... : different colours for different station types
%  - opt.fancy_legend .. : plot fancy_plot legend
%  - opt.plot_avg_yrange .. : fix the vertical range of the avg trend plot
%  - opt.plot_std_yrange .. : fix the vertical range of the std trend plot
%
% See also rio_init, rio_gettrend, rio_typeplot
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_plottrend( cnf, varargin )

if nargin > 1
    opt = varargin{1};    
    if ~isfield( opt, 'weekpart' ), opt.weekpart   = 'all';  end;
    if ~isfield( opt, 'show_plot' ), opt.show_plot = true;   end;
    if ~isfield( opt, 'save_plot' ), opt.save_plot = false;  end;
    if ~isfield( opt, 'plot_labels' ), opt.plot_labels = false;  end;
    if ~isfield( opt, 'label_dx' ), opt.label_dx = 0.01; end;
    if ~isfield( opt, 'label_dy' ), opt.label_dy = 0.50; end;
    if ~isfield( opt, 'label_fontsize' ), opt.label_fontsize = 7; end;
    if ~isfield( opt, 'fancy_plot' ), opt.fancy_plot = true; end;
    if ~isfield( opt, 'fancy_legend' ), opt.fancy_legend = true; end;
    
    if nargin == 8
        fprintf( 'rio_plottrend: trend parameter specified by user\n' );
        % get the trend parameters from the 
        xx_avg    = varargin{2};
        xx_std    = varargin{3};
        p_avg     = varargin{4};
        p_avg_err = varargin{5};
        p_std     = varargin{6};
        p_std_err = varargin{7};
    else
        fprintf( 'rio_plottrend: loading trend parameters from rio_param\n' );
        % read the trend parameters from the file
        tmp = load( fullfile( cnf.paramPath, 'trend', ...
            sprintf( 'avg_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, ...
            cnf.gis_type, opt.weekpart, cnf.at_lb ) ), 'p_avg');
        p_avg = tmp.p_avg;
        
        tmp = load( fullfile( cnf.paramPath, 'trend', ...
            sprintf( 'avg_err_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, ...
            cnf.gis_type, opt.weekpart, cnf.at_lb ) ), 'p_avg_err');
        p_avg_err = tmp.p_avg_err;
        
        tmp = load( fullfile( cnf.paramPath, 'trend', ...
            sprintf( 'std_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, ...
            cnf.gis_type, opt.weekpart, cnf.at_lb ) ), 'p_std');
        p_std = tmp.p_std;
        
        tmp = load( fullfile( cnf.paramPath, 'trend', ...
            sprintf( 'std_err_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, ...
            cnf.gis_type, opt.weekpart, cnf.at_lb ) ), 'p_std_err');                
        p_std_err = tmp.p_std_err;
        
        % load mean & std...
        tmp = load( fullfile( cnf.paramPath, 'stat_param', ...
            sprintf( 'avg_%s_%s_agg_time-%s.mat',  cnf.pol_xx, ...
            opt.weekpart, cnf.at_lb ) ), 'xx_avg');
        xx_avg = tmp.xx_avg;
        
        tmp = load( fullfile( cnf.paramPath, 'stat_param', ...
            sprintf( 'std_%s_%s_agg_time-%s.mat',  cnf.pol_xx, ...
            opt.weekpart, cnf.at_lb ) ), 'xx_std');
        xx_std = tmp.xx_std;
    end
end

% only use these station indices, can be integrated in the cnf later on
% use all of them for now
st_list = 1:cnf.nr_st;
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
delta     = max( cnf.st_indic ) - min( cnf.st_indic );
edge      = opt.edgefactor * delta;
tmp_indic = min( cnf.st_indic )-edge:opt.indic_step:max(cnf.st_indic)+edge;

% -- Plotting for different aggregation times
if cnf.agg_time <= 3
    
    figure( 'Name','RIO trends', 'Position', [ 100 100 1000 400 ], ...
        'Visible', 'off' );
    % ---------------------------------------------------------------------
    % Plotting the avg trend
    % ---------------------------------------------------------------------
    subplot( 1, 2, 1 );    
    if opt.fancy_plot
        rio_typeplot( cnf.st_indic(st_idx), xx_avg(st_idx,2), cnf.st_info(st_idx, 5 ), opt.fancy_legend );
    else
        plot( cnf.st_indic(st_idx), xx_avg(st_idx,2), 'dr' );
    end
    
    if isfield( opt, 'plot_avg_yrange' )
        set( gca, 'YLim', opt.plot_avg_yrange )
    end
    set( gca, 'XLim', [ min(tmp_indic) max(tmp_indic) ] );
    hold on;
    
    if strcmp( opt.fitmode, 'polyfit' )
        trend_avg     = polyval( p_avg, tmp_indic );
        trend_avg_err = polyval( p_avg_err, tmp_indic );
    else
        trend_avg     = opt.fcn_avg( p_avg, tmp_indic );
        trend_avg_err = opt.fcn_avg_err( p_avg_err, tmp_indic );
    end
    
    plot( tmp_indic, trend_avg, '-' );
    plot( tmp_indic, trend_avg + trend_avg_err, 'g:' );
    plot( tmp_indic, trend_avg - trend_avg_err, 'g:' );
    
    if ( opt.plot_labels )
        text( cnf.st_indic(st_idx)+opt.label_dx, xx_avg(st_idx,2)+opt.label_dy, ...
            cnf.st_id(st_idx), 'Fontsize', opt.label_fontsize , 'Color', 'k' );
    end
    grid on;
    hold off;
    if cnf.Option.logtrans
      ylabel(['\langle ln( 1 + ', cnf.pol_xx, ') \rangle',' [ln(\mug/m^3)] - ', cnf.at_lb ]);
    else
      ylabel(['\langle', cnf.pol_xx, '\rangle',' [\mug/m^3] - ', cnf.at_lb ]);
    end
    xlabel( strrep( cnf.gis_type, '_', '\_' ) );
    
    % ---------------------------------------------------------------------
    % Plotting the std trend
    % ---------------------------------------------------------------------
    subplot( 1, 2, 2 );
    if opt.fancy_plot
        rio_typeplot( cnf.st_indic(st_idx), xx_std(st_idx,2), cnf.st_info(st_idx, 5 ), opt.fancy_legend );
    else
        plot( cnf.st_indic(st_idx), xx_std(st_idx,2), 'dr' );
    end
    
    if isfield( opt, 'plot_std_yrange' )
        set( gca, 'YLim', opt.plot_std_yrange )
    end
    set( gca, 'XLim', [ min(tmp_indic) max(tmp_indic) ] );
    hold on;
    
    if strcmp( opt.fitmode, 'polyfit' )
        trend_std     = polyval( p_std, tmp_indic );
        trend_std_err = polyval( p_std_err, tmp_indic );
    else
        trend_std     = opt.fcn_std( p_std, tmp_indic );
        trend_std_err = opt.fcn_std_err( p_std_err, tmp_indic );
    end
    
    plot( tmp_indic, trend_std, '-' );
    plot( tmp_indic, trend_std + trend_std_err, 'g:' );
    plot( tmp_indic, trend_std - trend_std_err, 'g:' );
    
    if ( opt.plot_labels )
        text( cnf.st_indic(st_idx)+opt.label_dx, xx_std(st_idx,2)+opt.label_dy, ...
            cnf.st_id(st_idx), 'Fontsize', opt.label_fontsize , 'Color', 'k' );
    end
    grid on;
    hold off;
    
    if cnf.Option.logtrans
      ylabel(['\sigma _{ ln( 1 + ', cnf.pol_xx, ') }',' [ln(\mug/m^3)] - ', cnf.at_lb ]);
    else
      ylabel(['\sigma _{', cnf.pol_xx, '}',' [\mug/m^3] - ', cnf.at_lb ]);
    end
    xlabel( strrep( cnf.gis_type, '_', '\_' ) );
    
    % ---------------------------------------------------------------------
    % Save/show the plot
    % ---------------------------------------------------------------------
    if opt.save_plot
        fname = fullfile( cnf.outputPath, sprintf( 'rio_trend_avg_%s_%s_agg_time-%s.emf', ...
            cnf.pol_xx, opt.weekpart, cnf.at_lb ) );
        saveas( gcf, fname, 'emf' );
        fprintf( 'rio_plottrend: saved plot %s\n',  fname );
    end
    
    if opt.show_plot
        set( gcf, 'Visible', 'on' ); % & leave the handle open
    else
        delete( gcf );
    end

else    
    % ---------------------------------------------------------------------
    %  Plotting the 24 avg trend values...
    % ---------------------------------------------------------------------
    figure( 'Name','RIO avg trends', 'Position', [ 100 100 1200 800 ] ); %, ...
        %'Visible', 'off' );
    
    for k=1:24
        subplot( 4, 6, k );
        if opt.fancy_plot
            if ( k == 1 ), lg = opt.fancy_legend; else lg = false; end                
            rio_typeplot( cnf.st_indic(st_idx), xx_avg(st_idx,k+1), cnf.st_info(st_idx, 5 ), lg );
        else
            plot( cnf.st_indic(st_idx), xx_avg(st_idx,k+1), 'dr' );            
        end
        
        if isfield( opt, 'plot_avg_yrange' )
            set( gca, 'YLim', opt.plot_avg_yrange )
        end
        set( gca, 'XLim', [ min(tmp_indic) max(tmp_indic) ] );
        hold on;
        
        if strcmp( opt.fitmode, 'polyfit' )
            trend_avg     = polyval( p_avg(k,:), tmp_indic );
            trend_avg_err = polyval( p_avg_err(k,:), tmp_indic );
        else
            trend_avg     = opt.fcn_avg( p_avg(k,:), tmp_indic );
            trend_avg_err = opt.fcn_avg_err( p_avg_err(k,:), tmp_indic );
        end
        
        plot( tmp_indic, trend_avg, '-' );
        plot( tmp_indic, trend_avg + trend_avg_err, 'g:' );
        plot( tmp_indic, trend_avg - trend_avg_err, 'g:' );        
        if ( opt.plot_labels )
            text( cnf.st_indic(st_idx)+opt.label_dx, xx_avg(st_idx,k+1)+opt.label_dy, ...
                cnf.st_id(st_idx), 'Fontsize', opt.label_fontsize , 'Color', 'k' );
        end
        hold off;
        if cnf.Option.logtrans
          ylabel( sprintf( '\\langle ln( 1 + %s ) \\rangle - %02dh', cnf.pol_xx, k-1 ) );
        else
          ylabel( sprintf( '\\langle%s\\rangle - %02dh', cnf.pol_xx, k-1 ) );
        end
        xlabel( strrep( cnf.gis_type, '_', '\_' ) );
        
    end    
        
    % ---------------------------------------------------------------------
    %  Plotting the 24 std trend values...
    % ---------------------------------------------------------------------
    figure( 'Name','RIO std trends', 'Position', [ 100 100 1200 800 ] ); %, ...
        %'Visible', 'off' );
    
    for k=1:24
        subplot( 4, 6, k );
        if opt.fancy_plot
            if ( k == 1 ), lg = opt.fancy_legend; else lg = false; end                
            rio_typeplot( cnf.st_indic(st_idx), xx_std(st_idx,k+1), cnf.st_info(st_idx, 5 ), lg );
        else
            plot( cnf.st_indic(st_idx), xx_std(st_idx,k+1), 'dr' );            
        end 
        
        if isfield( opt, 'plot_std_yrange' )
            set( gca, 'YLim', opt.plot_std_yrange )
        end
        set( gca, 'XLim', [ min(tmp_indic) max(tmp_indic) ] );
        hold on;
        
        if strcmp( opt.fitmode, 'polyfit' )
            trend_std     = polyval( p_std(k,:), tmp_indic );
            trend_std_err = polyval( p_std_err(k,:), tmp_indic );
        else
            trend_std     = opt.fcn_std( p_std(k,:), tmp_indic );
            trend_std_err = opt.fcn_std_err( p_std_err(k,:), tmp_indic );
        end
        
        plot( tmp_indic, trend_std, '-' );
        plot( tmp_indic, trend_std + trend_std_err, 'g:' );
        plot( tmp_indic, trend_std - trend_std_err, 'g:' );
        if ( opt.plot_labels )
            text( cnf.st_indic(st_idx)+opt.label_dx, xx_std(st_idx,k+1)+opt.label_dy, ...
                cnf.st_id(st_idx), 'Fontsize', opt.label_fontsize , 'Color', 'k' );
        end
        hold off;
        if cnf.Option.logtrans
          ylabel( sprintf( '\\sigma_{ ln( 1 + %s ) }- %02dh', cnf.pol_xx, k-1 ) );
        else
          ylabel( sprintf( '\\sigma_{%s}- %02dh', cnf.pol_xx, k-1 ) );
        end
        xlabel( strrep( cnf.gis_type, '_', '\_' ) );        
    end  
    
end




