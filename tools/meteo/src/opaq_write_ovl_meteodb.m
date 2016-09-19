%OPAQ_WRITE_OVL_METEODB Writes a NEW OVL mat file with the dates & meteo
%
% Usage:
%   opaq_write_ovl_meteodb( fname, xx_date_all, xx_meteo_all )
%
% Arguments
%   - fname .......... : filename for the OVL .mat database
%   - xx_date_all .... : column vector of the dates
%   - xx_meteo_all ... : matrix of the meteo parameters, one date entry for
%                        every row, the number of columns gives the
%                        different variables...
%
% Optional parameters
%  'timeZone', <string> : string to identify the timezone
%  'gfsBase' datenum    : set the utc basetime for this particular update
%                         normally only with GFS...
%
% Bino Maiheu, (c) 2014 VITO

function xx_meteo = opaq_write_ovl_meteodb( fname, xx_date_all, xx_meteo_all, varargin )

% some input parameter parsing...
ip = inputParser();
addRequired(ip, 'fname', @isstr );
addRequired(ip, 'xx_date_all', @iscolumn );
addRequired(ip, 'xx_meteo_all', @ismatrix );
addParamValue( ip, 'timeZone', 'UTC');
addParamValue( ip, 'gfsBase', 0 );
parse(ip,fname,xx_date_all,xx_meteo_all,varargin{:});


% extract the hours and create an hours vector...
dv = datevec(xx_date_all);
xx_hours = sort(unique(dv(:,4)));

% sanity check
if numel(xx_hours) ~= 4
    % meteo modesl considered so far have 6hourly data values, put in a
    % warning if found otherwise...    
    warning( '++++ We did not find 4 hours in the meteo dataset.. bizarre...' );
else
    fprintf( 'Exporting dat in %s, hours present : ', ip.Results.timeZone );
    fprintf( ' %02d', xx_hours );
    fprintf( '\n' );
end

% create start date
start_date = min(floor(xx_date_all));
end_date   = max(floor(xx_date_all));
exp_dates  = start_date:datenum(0,0,1):end_date;
num_vars   = size(xx_meteo_all,2);

% pre allocate the output meteo
xx_meteo   = nan(length(exp_dates)*num_vars,2+length(xx_hours));
   
for p=1:num_vars % loop over the parameters

    offset = (p-1)*length(exp_dates);
       
    for i_date=1:length(exp_dates)
        xx_meteo(offset+i_date,1) = exp_dates(i_date); % the day
        xx_meteo(offset+i_date,2) = p;                 % the parameter number
           
        for i_hour=1:length(xx_hours)
            idx = find( abs(xx_date_all - (exp_dates(i_date)+datenum(0,0,0,xx_hours(i_hour),0,0) ) ) < 1e-6 );
            if isempty(idx)
                xx_meteo(offset+i_date,2+i_hour) = NaN;
            elseif length(idx) == 1
                xx_meteo(offset+i_date,2+i_hour) = xx_meteo_all(idx,p);
            else
                error( 'Multiple entries detected... this should not happen..' );
            end
                   
        end
           
    end
       
end
fprintf( 'Saving %s...\n', fname );     

% some additional parameters
tzone    = ip.Results.timeZone;
gfs_base = ip.Results.gfsBase;
save( fname, 'xx_meteo', 'xx_hours', 'tzone', 'gfs_base' );
   
