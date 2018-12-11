% ------------------------------------------------------------------------
%  Get time windows


%% Config
pol_xx = 'so2';
%xx_input_file =  '.\data\dbq_20120905_RIO_PM10_All.dat';
%xx_input_file =  '.\data\dbq_20120905_RIO_PM25_All.dat';
%xx_input_file =  '.\data\dbq_20120905_RIO_NO2_All.dat';
%xx_input_file =  '.\data\dbq_20120905_RIO_O3_All.dat';
xx_input_file =  '.\data\so2_data.rio.txt';

%% Gett he info
fprintf( 'Getting measurement info from %s...\n', xx_input_file );
fid  = fopen(xx_input_file, 'r');
if fid == -1 
    error( 'Cannot open file %s', xx_input_file );
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
    
    % Trow out the luxemburg stations.
    if strncmp( 'LUX', station_id, 3 ), 
        istr = fgetl( fid );
        fprintf( 'Skippig station %s...\n', station_id );
        continue; 
    end;
    
    % Throw out 41LEC1 and 41LEB2 etc...
    if strncmp( '41LE', station_id, 4 ),
        istr = fgetl( fid );
        fprintf( 'Skippig station %s...\n', station_id );
        continue; 
    end;
        
    
    % Field names cannot start with a digit in matlab... GRRRRR !!!!
    if ~isfield( st_list, [ 'ST', station_id ] )
        st_list.(['ST', station_id]).start_date = xx_date;
    end
    
    % Update end date
    st_list.(['ST', station_id]).end_date = xx_date;
   
    istr = fgetl( fid );
end

fclose(fid);


%% Gewesten laden
fid = fopen( 'D:/Projects/N7862/2012/stations_gewesten.txt', 'r' ); 
gew = textscan( fid, '%s%d%s', 'headerlines', 3, 'delimiter', ';' ); 
fclose(fid);

%% Stations types laden
fid = fopen( 'D:\Projects\N7862\2012\stations.txt', 'r' );
typ = textscan( fid, '%s%d%d%d%s', 'delimiter', '\t' );
fclose(fid);

%% Output file
xx_output_file = ['timewindow_',pol_xx,'_report.txt'];



%% Define colors
fn = fieldnames( st_list );
vito_orange = [245 130 32]/255;
vito_blue   = [52 163 220]/255;
vito_green  = [103 175 62]/255;
vito_black  = [35 31 32]/255;

%%
figure( 'Position', [ 50 100 1600 900 ] );
axis( [ datenum( 1999,12,31,20,0,0) datenum(2012,1,1,4,0,0) 0.5 length(fn)+0.5  ] );
set( gca, 'Box', 'on' );
w=0.6;
fid = fopen( xx_output_file, 'w');
fprintf( fid, 'Timewindows for %s data\n', pol_xx );
fprintf( fid, 'Based upon %s\n', xx_input_file );
col_by_type = true;

for i=1:length(fn),
    st_id = regexprep(fn{i}, '^ST', ''); % get rid of the ST

    typ_id = find( strcmp( typ{:,1}, regexprep(fn{i}, '^ST', '') ) );
    gew_id = find( strcmp( gew{:,1}, regexprep(fn{i}, '^ST', '') ) );
    
    
    if col_by_type
        % set color by station type        
        if isempty( typ_id )
            col = [ 0.7 0.7 0.7 ]; % gray
        else
           switch typ{2}(typ_id)
               case 1, % rural
                   col = vito_green;
               case 2, % urban background
                   col = [ 1.0000    1.0000    0.5000 ];
               case 3, % urban
                   col = vito_orange;
               case 4, % industrial
                   col = vito_blue;
               case 5, % traffic
                   col = vito_black;
               otherwise,
                   col = [ 0.7 0.7 0.7 ];           
           end
        end
    else
        % set color by station region
        if isempty( gew_id )
            col = vito_black;
        else
            switch gew{2}(gew_id)
                case 1, % vlaanderen
                    col = vito_orange;
                case 2, % wallonie
                    col = vito_green;
                case 3, % brussel
                    col = vito_blue;
            end
        end
    end
    
    % print a line
    fprintf( fid, '%d \t %s \t %d \t %d \t %d \t %s --> %s\n', i, st_id, ...
        typ{3}(typ_id), typ{4}(typ_id), typ{2}(typ_id), ...
        datestr( st_list.(fn{i}).start_date), datestr( st_list.(fn{i}).end_date ) );
    
    rectangle( 'Position', [ st_list.(fn{i}).start_date  i-w/2 st_list.(fn{i}).end_date-st_list.(fn{i}).start_date w ], ...
        'FaceColor', col );
end
fclose(fid);

%% Finish the plot...
datetick('x', 'keeplimits' );
set(gca,'YTick', 1:length(fn));
set(gca,'YTickLabel', regexprep( fn, '^ST', '' ), 'FontSize', 7, 'FontName', 'Calibri' );
grid on;

xlabel( 'Jaar', 'FontName', 'Calibri', 'FontSize', 12 );
title( sprintf( 'Data beschikbaarheids periode per station, %s', pol_xx ), 'FontName', 'Calibri', 'FontSize', 12 );

