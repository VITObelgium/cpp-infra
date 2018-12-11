%RIO_CREATEDB
% Function to create (bootstrap) a historic database from a datafile
% for the setup defined in the config structure. If no filename is 
% given, one will be requested via a dialog...
%  
%  rio_createdb( cnf )
%  rio_createdb( cnf, fname )
%  rio_createdb( cnf, fname, save_mat )
%
% Note that by default, the createdb routine does not load the database, 
% you need to do this with rio_loaddb unless the save_mat optional argument
% is false, in this case, the matlab files are not saved but the
% concentrations are directly loaded into the configuration structure. 
%
% See also rio_loaddb, rio_init, rio_checkdeployment
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function cnf = rio_createdb( cnf, varargin )

save_mat = true;
if nargin > 1   
   fname = varargin{1};  
   if nargin > 2
       save_mat = varargin{2};
   end
else
   % read the filename from a dialog
   [filename, pathname] = uigetfile( {'*.txt;*.asc;*.dat;*.dbq', ...
       'ASCII files (*.txt, *.asc, *.dat, *.dbq)'; ...
       '*.*', 'All Files (*.*)'}, sprintf( 'Select the %s datafile...', cnf.pol ) );   
   
   if ~( isequal(filename,0) || isequal(pathname,0) )
       fname = fullfile( pathname, filename );
   else
       fprintf( '++ rio_createdb cancelled by user...\n' );
       return
   end
end

if ~exist( fname, 'file' )
   fprintf( '++ rio_createdb:: no such file %s\n', fname );
   return;
end

% logging info
rio_log( sprintf( 'creating %s database from %s', cnf.pol, fname ) );

% does the folder exist ?
if ~exist( cnf.dbasePath, 'dir' )
    rio_log( sprintf( 'creating data folder %s', cnf.dbasePath ) );
    fprintf( 'Creating data folder %s', cnf.dbasePath );
    mkdir( cnf.dbasePath );
end

% Now open the datafile
switch( cnf.deployment )
    
    case { 'VITO', 'IRCEL' }
        %-- here we have the dbq output file style...    
        %-- Define variables
        file_max_txt   = fullfile( cnf.dbasePath, sprintf( '%s_data_max.txt', cnf.pol_xx ) );       
        file_max_mat   = fullfile( cnf.dbasePath, sprintf( '%s_data_max.mat', cnf.pol_xx ) );
        file_1h_txt    = fullfile( cnf.dbasePath, sprintf( '%s_data_1h.txt', cnf.pol_xx ) );
        file_1h_mat    = fullfile( cnf.dbasePath, sprintf( '%s_data_1h.mat', cnf.pol_xx ) );
        file_dates_mat = fullfile( cnf.dbasePath, sprintf( '%s_days.mat', cnf.pol_xx ) );

        %-- open the datafile...
        fid = fopen( fname, 'r');
        
        %-- open the output txt files...
        fid_out_max = fopen(file_max_txt, 'w');
        fid_out_1h  = fopen(file_1h_txt, 'w');

        
        %-- Initialize
        MAX_LN = 300000;
        xx_val_max = zeros(MAX_LN, 4);
        xx_val_1h = zeros(MAX_LN, 25);
        xx_date = zeros(MAX_LN,1);
        
        %-- Loop over file
        istr = fgetl(fid);
        ln_cnt = 0;
        st_cur = '';
        while istr ~= -1
            station_id = istr(1:6);
            if ~strcmp(station_id, st_cur)
                st_cur = station_id;                
            end
            
            xx_array = str2num(istr(7:end));

            if strcmp( cnf.pol, 'bc' ) % if we deal with BC, divide by 100 !!!, but beware of -9999 !!!
                my_arr = xx_array(2:end);
                my_arr( my_arr > 0 ) = 0.01 .* my_arr( my_arr > 0 );
                xx_array(2:end) = my_arr;
            end
            
            %-- Find internal station no...
            i = find(strcmp(cnf.st_id, cellstr(station_id)));                
            
            if not(isempty(i))
                ln_cnt = ln_cnt + 1;
                if ln_cnt > MAX_LN
                    error( 'rio_createdb:: line count > MAX_LN !!' );
                end
        
                %-- Write txt file for statistics (max1h, max8h, day avg)...
                fprintf(fid_out_max, '%s\t', station_id);
                fprintf(fid_out_max, '%d\t', i);
                fprintf(fid_out_max, '%d\t%d\t%d\t%d', xx_array(1:4));
                fprintf(fid_out_max, '\n');
        
                %-- Write txt file for 1h values...
                fprintf(fid_out_1h, '%s\t', station_id);
                fprintf(fid_out_1h, '%d\t', i);
                fprintf(fid_out_1h, '%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d', ...
                    xx_array(1), xx_array(5:end));
                fprintf(fid_out_1h, '\n');
        
                %-- Build export matrix for statistics...
                dd = xx_array(1);
                yy = round(dd/10000);
                mm = round((dd - yy*10000)/100);
                dd = dd - yy*10000 - mm*100;
        
                xx_date(ln_cnt,:) = datenum(yy, mm, dd);
                xx_val_max(ln_cnt,:) = [i, xx_array(2:4)];
                xx_val_1h(ln_cnt,:) = [i, xx_array(5:end)];
            end
    
            istr = fgetl(fid);
        end

        fclose( fid );
        fclose( fid_out_1h );
        fclose( fid_out_max );
        
        %-- Trim...
        xx_date(ln_cnt+1:end,:)    = [];
        xx_val_max(ln_cnt+1:end,:) = [];
        xx_val_1h(ln_cnt+1:end,:)  = [];
    
        %-- Export....
        if save_mat
            save( file_max_mat,   'xx_val_max' );
            save( file_1h_mat,    'xx_val_1h' );
            save( file_dates_mat, 'xx_date' );
                        
            fprintf( 'Historic database stored in %s, %s and %s\n', ...
                file_dates_mat, file_1h_mat, file_max_mat );   
        else
            if cnf.agg_time <= 3
                cnf.xx_val = xx_val_max;
            else
                cnf.xx_val = xx_val_1h;
            end
            cnf.xx_date = xx_date;            
            cnf.xx_val( cnf.xx_val == -9999 ) = NaN;
            cnf.have_db = true;
            fprintf( 'Database loaded in configuration...\n' );
        end
        
    
        
    % case { 'RIO-NH3' }       
    otherwise
      error( 'rio_createdb:: unknown deployment %s', cnf.deployment );  
end


end