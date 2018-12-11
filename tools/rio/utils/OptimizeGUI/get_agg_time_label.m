function at_lb = get_agg_time_label(agg_time)

%-- Return aggregation time label for use in parameter files

switch agg_time
    case 1
        at_lb = 'm1';
    case 2
        at_lb = 'm8';
    case 3
        at_lb = 'da';
    case 4
        at_lb = '1h';
end

      