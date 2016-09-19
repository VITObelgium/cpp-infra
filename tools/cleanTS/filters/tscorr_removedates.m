function [ xx_date_new, xx_vals_new ] = tscorr_removedates( xx_date, xx_vals, opts )

xx_date_new = xx_date;
if ~isfield(opts, 'start_date'), opts.start_date = inf; end;
if ~isfield(opts, 'end_date'), opts.end_date = -inf; end;

xx_vals_new = xx_vals;
xx_vals_new( xx_date >= opts.start_date & xx_date <= opts.end_date ) = NaN;