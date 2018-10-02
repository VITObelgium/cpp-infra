%RIO_H5APPEND
%
% Appends the current interpolated output to the hdf5 file, setting the
% time stamp in the /Time group, the concentration and uncertainty in the 
% /Grid group and the stations values in the /Stations group. 
% 
%  fh = rio_h5append( cnf, fh, date, vals, grid )
%
% It's important to catch the returned filehandle in order to have the
% index in the file be incremented. 
%
% See also rio_init, rio_h5close, rio_h5create
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function fh = rio_h5append( cnf, fh, date, vals, grid )


%% set -9999 to be missing value in the output
vals( vals == cnf.missing_value ) = -9999;

%% append the grid
offset = [ 0 fh.curr_idx ];
count  = [ cnf.grid_n 1 ];
if fh.curr_idx > 0       
    % extend the output from the second index onwards
    H5D.extend( fh.concID,[ cnf.grid_n fh.curr_idx+1 ] );
    H5D.extend( fh.errorID,[ cnf.grid_n fh.curr_idx+1 ] );
    
    % and get it's space back
    fh.grid_space = H5D.get_space( fh.concID );    
end

H5S.select_hyperslab( fh.grid_space, ...
    'H5S_SELECT_SET', offset, [], count, [] );    
H5D.write( fh.concID, H5T.copy( 'H5T_NATIVE_FLOAT' ), fh.grid_memspaceID, ...
    fh.grid_space, 'H5P_DEFAULT', cast( grid(:,2), 'single' ) );    
H5D.write( fh.errorID, H5T.copy( 'H5T_NATIVE_FLOAT' ), fh.grid_memspaceID, ...
    fh.grid_space, 'H5P_DEFAULT', cast( grid(:,3), 'single' ) );

%% append the stations
count  = [ cnf.nr_st 1 ];
if fh.curr_idx > 0       
    H5D.extend( fh.st_concID,[ cnf.nr_st fh.curr_idx+1 ] );
    fh.stat_space = H5D.get_space( fh.st_concID );    
end
H5S.select_hyperslab( fh.stat_space, ...
    'H5S_SELECT_SET', offset, [], count, [] );    
H5D.write( fh.st_concID, H5T.copy( 'H5T_NATIVE_FLOAT' ), fh.stat_memspaceID, ...
    fh.stat_space, 'H5P_DEFAULT', cast( vals(:,2), 'single' ) );    

%% append the current timestamp
offset = fh.curr_idx;
count  = 1;
if fh.curr_idx > 0       
    H5D.extend( fh.yearID, fh.curr_idx+1 );
    H5D.extend( fh.monthID, fh.curr_idx+1 );
    H5D.extend( fh.dayID, fh.curr_idx+1 );
    if cnf.agg_time == 4
        H5D.extend( fh.hourID, fh.curr_idx+1 );
    end
    fh.time_space = H5D.get_space( fh.yearID );    
end
H5S.select_hyperslab( fh.time_space, ...
    'H5S_SELECT_SET', offset, [], count, [] );    

v = datevec( date );
H5D.write( fh.yearID, 'H5T_NATIVE_INT16', fh.time_memspaceID, ...
    fh.time_space, 'H5P_DEFAULT', cast( v(1), 'int16') ); 
H5D.write( fh.monthID, 'H5T_NATIVE_INT16', fh.time_memspaceID, ...
    fh.time_space, 'H5P_DEFAULT', cast( v(2), 'int16') ); 
H5D.write( fh.dayID, 'H5T_NATIVE_INT16', fh.time_memspaceID, ...
    fh.time_space, 'H5P_DEFAULT', cast( v(3), 'int16') ); 
if cnf.agg_time == 4
    H5D.write( fh.hourID, 'H5T_NATIVE_INT16', fh.time_memspaceID, ...
        fh.time_space, 'H5P_DEFAULT', cast( v(4) + 1, 'int16') ); 
end


%% Increment the index
fh.curr_idx = fh.curr_idx + 1;