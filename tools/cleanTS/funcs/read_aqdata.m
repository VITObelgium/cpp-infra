%READ_AQDATA reads air quality data for different fileformats
% 
% Usage: 
%    [ xx_date, xx_vals ] = read_aqdata( fname, [options ] )
%
% Parameters : 
%
%  'Format', '....' : specify the file format, supported are : 
%            * 'libovito'  : the LIBOVITO 24 hours ascii file in CST
%             
%  'Reshape1D', true/false : if true then the data is reshaped to a 1D 
%               array instead of e.g. 24 hourly values in 2D matrix
%
%  'Timestamp', 'After'/'Before' : assign when reshaping to 1D array
%               the hourly timestamps before or after the corresponding
%               value. I.e. when using after, we assing datenum(0,0,0,1,0,0)
%               to the first measurement of hte day, otherwise datenum(0,0,0,0,0,0)
%               so in the first case we treat the measurment of a specific
%               timestamp as the aggregation of the past interval(hour) before
%               that timestamp.
%
%  'Trim', true/false : remove trailing & leading NaN from timeseries, at
%               the moment only works in Reshape1D mode
%
%  
% Bino Maiheu, (c) 2014 VITO
% Contact : bino.maiheu@vito.be


function [ xx_date, xx_vals ] = read_aqdata( fname, varargin )

% parse arguments
p = inputParser;
p.addRequired( 'fname', @(x)validateattributes(x,{'char'}, {'nonempty'} ) );
p.addParamValue( 'Format', 'libovito', @(x)validateattributes(x,{'char'}, {'nonempty'}) );
p.addParamValue( 'Reshape1D', false, @islogical );
p.addParamValue( 'Trim', false, @islogical );
p.addParamValue( 'Timestamp', 'After', @(x)validateattributes(x,{'char'}, {'nonempty'}) );
p.parse( fname, varargin{:} );
opts = p.Results;

fid = fopen( fname, 'r' );
if fid < 0
    error( 'read_aqdata: Unable to open file for reading: %s', fname );    
end

switch lower( opts.Format )
    
    % =====================================================================
    %  LIBOVITO ASCII FILE FORMAT
    % =====================================================================
    case 'libovito';
        try
            fprintf( 'Scanning...\n' );
            C = textscan( fid, [ '%s\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f' ...
                '\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f' ]  );
            xx_date = cellfun( @datenum, C{1,1} );
            xx_vals = cell2mat( {C{1,2:end}} );
        catch MEx
            fprintf( 'read_aqdata: %s\n', MEx.message );
            fclose(fid);
            return;
        end
        
        % convert the timestamp : CST to UTC : offset is 8 hours (CST is ahead)
        % [ xx_date, xx_vals ] = date_shift( xx_date, xx_vals, 8 );
        % we left it at CST and shifted the meteo data to UTC...

        % handle NaN
        xx_vals(xx_vals ==  9999 ) = NaN;
        xx_vals(xx_vals == -9999 ) = NaN;
        xx_vals(xx_vals < 0 ) = NaN;
        xx_vals = xx_vals .* 1000; % express in µg/m3
        
      
        
        % reshape to 1D ??
        if opts.Reshape1D
            fprintf( 'Reshaping to 1D timeseries, timestamp %s the hour...\n', lower(opts.Timestamp) );
            switch lower( opts.Timestamp )
                case 'after'
                    hrs = datenum(0,0,0,1,0,0):datenum(0,0,0,1,0,0):datenum(0,0,1,0,0,0);
                case 'before'
                    hrs = datenum(0,0,0,0,0,0):datenum(0,0,0,1,0,0):datenum(0,0,0,23,0,0);
                otherwise
                    fclose(fid);
                    error( 'read_aqdata: Invalid timestamp paramet specification' );  
            end
            xx_date = repmat( xx_date, 1, 24 ) + repmat( hrs, size(xx_date,1), 1 );
            xx_date = reshape( xx_date', 1, numel(xx_date) )';
            xx_vals = reshape( xx_vals', 1, numel(xx_vals))';
            
            if opts.Trim
                fprintf( 'Trimming timeseries...\n' );
                i1 = find( ~isnan(xx_vals), 1, 'first');
                if ( i1 > 1 )
                    xx_vals(1:i1-1) = [];
                    xx_date(1:i1-1) = [];
                end
                
                i2 = find( ~isnan(xx_vals), 1, 'last' );
                if ( i2 < length(xx_vals) )
                    xx_vals(i2+1:end) = [];
                    xx_date(i2+1:end) = [];
                end
            end            
            
        else
            if opts.Trim
                warning( 'Skipping the trimming, only works in Reshape1D mode for now' );
            end
        end
        
        
    otherwise
        fclose(fid);
        error( 'read_aqdata: Format %s not supported.', opts.Format );
end

fclose(fid);
fprintf( 'Done.\n' );