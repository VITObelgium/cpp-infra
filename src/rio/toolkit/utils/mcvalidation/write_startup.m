function write_startup( fname, xx_val)

fid = fopen( fname, 'wt' );

fprintf( fid, '[MODEL]\n' );
fprintf( fid, ';Year\n' );
fprintf( fid, ';frequency\n' );
fprintf( fid, ';Scale\n' );
fprintf( fid, '2009\n' );
fprintf( fid, 'hour\n' );
fprintf( fid, 'urban\n' );
fprintf( fid, '[PARAMETERS]\n' );
fprintf( fid, ';Specie*type*measure unit\n' );
fprintf( fid, 'PM10;POL;ugm-3\n' );
fprintf( fid, '[MONITORING]\n' );
fprintf( fid, 'Station Code;Station Name;Station abbreviation;Altitude;Lon;Lat;GMTLag;Region;Station Type;Area Type;Siting;listOfvariables\n' );

for k=1:length(xx_val)
    
    switch( xx_val(k).type )
        case 0, str_type = 'unknown';
        case 1, str_type = 'rural';
        case 2, str_type = 'suburban';
        case 3, str_type = 'urban';
        case 4, str_type = 'industry';
        case 5, str_type = 'traffic';            
    end
    
fprintf( fid, '%s;%s;%s;0.;%f;%f;Belgium;Unknown;%s;Unknown;PM10*;\n', ...
    xx_val(k).name, xx_val(k).name, xx_val(k).name(3:end), ...
    xx_val(k).x, xx_val(k).y, str_type );

end

fclose(fid);