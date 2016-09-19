function opaq_write_ovl_ascii_meteo( base_name, xx_date_all, xx_meteo_all )

% -- Extract the hours and create an hours vector...
dv = datevec(xx_date_all);
xx_hours = sort(unique(dv(:,4)));

% create start date
start_date = min(floor(xx_date_all));
end_date   = max(floor(xx_date_all));
exp_dates  = start_date:datenum(0,0,1):end_date;
num_vars   = size(xx_meteo_all,2);
   
for p=1:num_vars % loop over the parameters

    fname = sprintf( '%s_P%02d.txt', base_name, p );
    fid = fopen( fname, 'wt' ); 
    if fid < 0
        error( 'Cannot open %s...', fname );
    end
    
    for i_date=1:length(exp_dates)
        fprintf( fid, '%s', datestr( exp_dates(i_date), 'yyyymmdd') );
                           
        for i_hour=1:length(xx_hours)
            idx = find( abs(xx_date_all - (exp_dates(i_date)+datenum(0,0,0,xx_hours(i_hour),0,0) ) ) < 1e-6 );
            if isempty(idx)
                fprintf( fid, '%14f', NaN );
            elseif length(idx) == 1
                fprintf( fid, '%14f', xx_meteo_all(idx,p) );
            else
                error( 'Multiple entries detected... this should not happen..' );
            end
                   
        end
        fprintf( fid, '\n' );
           
    end
    fclose(fid);
    fprintf( 'Wrote %s...\n', fname ),    
end
