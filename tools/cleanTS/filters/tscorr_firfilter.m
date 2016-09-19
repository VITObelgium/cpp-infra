function [ xx_date_new, xx_vals_new ] = tscorr_firfilter( xx_date, xx_vals, opts )

if ~isfield( opts, 'width'), opts.width = 3; end;
if ~isfield( opts, 'type'), opts.type = 'moving_average'; end;

switch lower(opts.type)
    case 'moving_average',
        if mod(opts.width,2) ~= 1
            errordlg( 'Need odd filter width for moving average', 'FIR Filter correction', 'modal' );
            
        end
        h = ones(1,opts.width)./opts.width;
        fprintf( 'FIR coefficients : ' ), fprintf( ' %.3f', h ); fprintf('\n');
    otherwise
        errordlg( 'Unknown filter type', 'FIR Filter correction', 'modal' );
end

% apply convolution
xx_vals_new = conv(xx_vals,h,'same');
xx_date_new = xx_date;