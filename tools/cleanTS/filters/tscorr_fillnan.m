function [ xx_date_new, xx_vals_new ] = tscorr_fillnan( xx_date, xx_vals, opts )

if ~isfield( opts, 'method' ), opts.method = 'interp1_nearest'; end;

xx_date_new = xx_date;
xx = xx_vals;

switch opts.method,
    case 'nearest'; 
        xx(isnan(xx)) = interp1(find(~isnan(xx)), xx(~isnan(xx)), find(isnan(xx)),'nearest');
    case 'interp1_linear'
        xx(isnan(xx)) = interp1(find(~isnan(xx)), xx(~isnan(xx)), find(isnan(xx)),'linear');
    case 'interp1_spline'
        xx(isnan(xx)) = interp1(find(~isnan(xx)), xx(~isnan(xx)), find(isnan(xx)),'spline');
    case 'interp1_cubic'
        xx(isnan(xx)) = interp1(find(~isnan(xx)), xx(~isnan(xx)), find(isnan(xx)),'cubic');
    case 'inpaint_nans_0'
        xx = inpaint_nans(xx, 0);
    case 'inpaint_nans_1'
        xx = inpaint_nans(xx, 1);
    case 'inpaint_nans_3'
        xx = inpaint_nans(xx, 3);
    case 'inpaint_nans_4'
        xx = inpaint_nans(xx, 4);        
    otherwise
        errordlg( 'Invalid method', 'Timeseries NaN filling correction', 'modal' );
end

xx_vals_new = xx;