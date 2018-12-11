function st_list = rioDataAnalyser_GetTimeWindows( pol_xx, fname )

fid  = fopen(fname, 'r');
if fid == -1 
    error( 'Cannot open file %s', fname );
    return;
end
istr    = fgetl(fid);
st_list = struct();

while ( istr ~= -1 )
    station_id = istr(1:6);
    
    xx_array = str2num(istr(7:end));
    
    % get date
    dd = xx_array(1);
    yy = round(dd/10000);
    mm = round((dd - yy*10000)/100);
    dd = dd - yy*10000 - mm*100;
    
    xx_date = datenum(yy, mm, dd); 
               
    % Field names cannot start with a digit in matlab... GRRRRR !!!!
    if ~isfield( st_list, [ 'ST', station_id ] )
      fprintf( 'Discovered station %s...\n', station_id );  
      st_list.(['ST', station_id]).start_date = xx_date;
      st_list.(['ST', station_id]).end_date = xx_date;
    end
    
    % Update end date
    if xx_date < st_list.(['ST', station_id]).start_date
      st_list.(['ST', station_id]).start_date = xx_date;
    end
    if xx_date > st_list.(['ST', station_id]).end_date
      st_list.(['ST', station_id]).end_date = xx_date;
    end
   
    istr = fgetl( fid );
end

fclose(fid);

fn = fieldnames( st_list );



