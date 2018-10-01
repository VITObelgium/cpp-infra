%WRITE_AQDATA writes air quality data to different fileformats
% 
% Usage: 
%    write_aqdata( fname, xx_date, xx_vals, [options ] )
%
% Parameters : 
%
%  'Format', '....' : specify the file format, supported are : 
%            * 'libovito'  : the LIBOVITO 24 hours ascii file
%            * 'ovldb'     : OVL database file (*.mat), set aggregation by
%                            ovl_aggr
%             
%  'Timestamp', 'after'/'before' : assign when reshaping to 1D array
%               the hourly timestamps before or after the corresponding
%               value. I.e. when using after, we assing datenum(0,0,0,1,0,0)
%               to the first measurement of hte day, otherwise datenum(0,0,0,0,0,0)
%               so in the first case we treat the measurment of a specific
%               timestamp as the aggregation of the past interval(hour) before
%               that timestamp.
%   
% 'ovlAggr', 'avg/max8h/max1h' : use this aggregation when outputting OVL format,
%             default is avg (=da)
%
% 'selHours', true/false : use hour selection in aggregation, default is
%             not, in which case avg is da, max1h is m1 and max8h is m8, if
%             we use selHours = true, then avg means we take the average
%             between the two hours...
% 'hr1', 'hr2', hour indices to aggregate between if selHours is true
%
%
% 'stationIndex'  : index in OVL to apply to the station...
%
%
% Bino Maiheu, (c) 2014 VITO
% Contact : bino.maiheu@vito.be


function write_aqdata( fname, xx_date, xx_vals, varargin )

% parse arguments
p = inputParser;
p.addRequired( 'fname', @(x)validateattributes(x,{'char'}, {'nonempty'} ) );
p.addParamValue( 'Format', 'libovito', @(x)validateattributes(x,{'char'}, {'nonempty'}) );
p.addParamValue( 'Timestamp', 'after', @(x)validateattributes(x,{'char'}, {'nonempty'}) );
p.addParamValue( 'ovlAggr', 'da', @(x)validateattributes(x,{'char'}, {'nonempty'}) );
p.addParamValue( 'stationIndex', 0, @(x)validateattributes(x,{'numeric'}, {'nonempty'}) );
p.addParamValue( 'selHours', 0, @(x)validateattributes(x,{'logical'}, {'nonempty'}) );
p.addParamValue( 'hr1', 0, @(x)validateattributes(x,{'numeric'}, {'nonempty'}) );
p.addParamValue( 'hr2', 0, @(x)validateattributes(x,{'numeric'}, {'nonempty'}) );
p.parse( fname, varargin{:} );
opts = p.Results;

if size( xx_vals, 2 ) == 1
    if any(size(xx_date)-size(xx_vals))
        error( 'xx_date and xx_vals have different sizes...' );
    end
elseif size( xx_vals, 2 ) == 24 
    % convert to hourly timeseries if we have 24 columns,
    % and we assume the xx_date timestamps are simply the days themselves
    % to which the 24 values apply...
    if strcmpi( opts.Timestamp, 'after' )
        hr24 = datenum(0,0,0,1,0,0):datenum(0,0,0,1,0,0):datenum(0,0,1,0,0,0);
    else
        hr24 = datenum(0,0,0,0,0,0):datenum(0,0,0,1,0,0):datenum(0,0,0,23,0,0);
    end
    
    % convert to linear array
    tmp = repmat( xx_date, 1, 24 ) + repmat( hr24, size(xx_date,1), 1 );
    xx_date = reshape( tmp', numel(tmp), 1 );
    xx_vals = reshape( xx_vals', numel(xx_vals), 1 );    
else
   error( 'Invalid number of columns in xx_vals...' );
end


%% if we want to do a m8, we need to compute the 8h sliding average
if strcmp( opts.ovlAggr, 'max8h' )
    % compute 8 hour sliding average
    xx_vals8 = nan(size(xx_vals));
    for i=1:length(xx_vals8)
       
        idx = find( xx_date >= (xx_date(i)-datenum(0,0,0,7,0,0)) & xx_date <= xx_date(i) );
        xx_vals8(i) = nanmean( xx_vals(idx) );
        
    end
end


%% convert back to 24hrs per datenum ?
switch lower( opts.Format )
    case { 'libovito', 'ovldb' }
        % convert back to original 2D matrix form, fill with -9999
        % can we assume we have complete days ? probably not... so
        % let's correct for this...
        if strcmpi( opts.Timestamp, 'after' ) % after the hour
            d_start = floor( xx_date(1) - datenum(0,0,0,1,0,0) );
            d_end   = floor( xx_date(end) - datenum(0,0,1,0,0,0) );
        else % before hour
            d_start = floor( xx_date(1) );
            d_end   = floor( xx_date(end) );            
        end
       
        xx_date_exp = d_start:datenum(0,0,1,0,0,0):d_end;
        xx_date_exp = xx_date_exp(:);
        xx_vals_exp = -9999*ones(size(xx_date_exp,1),24);
        if strcmp( opts.ovlAggr, 'max8h' )
            xx_vals8_exp = -9999*ones(size(xx_date_exp,1),24);
        end
        
        for i=1:length(xx_date_exp)
            
            for j=1:24
                
                if strcmpi( opts.Timestamp, 'after' ) % after the hour
                    idx = find( abs( xx_date-(xx_date_exp(i)+datenum(0,0,0,j,0,0))) < 1e-6 );
                else
                    idx = find( abs( xx_date-(xx_date_exp(i)+datenum(0,0,0,j-1,0,0))) < 1e-6 );
                end
                
                if length(idx) > 1
                    error( 'multiple entries found... strange...' );
                end
                if ~isempty(idx)
                    xx_vals_exp(i,j) = xx_vals(idx);
                    if strcmp( opts.ovlAggr, 'max8h' )
                        xx_vals8_exp(i,j) = xx_vals8(idx);
                    end
                end
            end
        end
        
    otherwise
        % do nothing in terms of conversion
end




%% Write output file
switch lower( opts.Format )
    
    % =====================================================================
    %  WRITE LIBOVITO ASCII FILE FORMAT
    % =====================================================================
    case 'libovito'
        fid = fopen( fname, 'wt' );
        if ( fid < 0 )
            error( 'Cannot open new file...' );
        end
        fprintf( 'Saving %s...\n', fname );
        for i=1:length(xx_date_exp)
            fprintf( fid, '%s', datestr(xx_date_exp(i), 'yyyy-mm-dd') );
            for j=1:size(xx_vals_exp,2)
                if isnan(xx_vals_exp(i,j)) || xx_vals_exp(i,j) == -9999
                    fprintf( fid, '\t-9999' );
                else
                    fprintf( fid, '\t%.5f', xx_vals_exp(i,j)/1000 ); % convert back to mg/m3
                end
            end
            fprintf( fid, '\n' );
        end
        fclose(fid);
        
        
    % =====================================================================
    %  WRITE OVL MATLAB DATABASE FORMAT
    % =====================================================================
    case 'ovldb';
        
        % xx_date_exp and xx_vals_exp contain the data...
        xx_vals_exp( xx_vals_exp == -9999 ) = NaN;
                
        if opts.selHours
            h1 = opts.hr1;
            h2 = opts.hr2;
        else
            h1 = 1;
            h2 = 24;
        end
        
        
        switch opts.ovlAggr
            case 'avg'
                xx_aggr = nanmean(xx_vals_exp(:,h1:h2),2);
            case 'max1h'
                xx_aggr = nanmax(xx_vals_exp(:,h1:h2),2);
            case 'max8h'
                xx_aggr = nanmax(xx_vals8_exp(:,h1:h2),2);
            otherwise
                error( 'Not implemented yet...' );
        end
        
        xx_aggr( isnan( xx_aggr ) ) = -9999;
        xx_vals_exp( isnan( xx_vals_exp ) ) = -9999;
        
        % construct the export variable...
        export = [ opts.stationIndex*ones(size(xx_date_exp,1),1) ...
            xx_date_exp xx_aggr xx_vals_exp ];
        
        save( fname, 'export' );
        
    otherwise
        error( 'write_aqdata: Format %s not supported.', opts.Format );
end

fprintf( 'Done.\n' );