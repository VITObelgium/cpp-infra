%RIO_DBDAQL
% This routine performs & applies some data quality checks on the loaded 
% historic databases. Note that this routine depends on the deployment,
% which is not a nice feature and should be updated in the future...
%
%  cnf = rio_dbdaql( cnf )
% 
% Implemented checks are : 
% * IRCEL/VITO deployment 
%   - drop O3 for winter periods if pollutant is o3s
%   - get rid of period of high PM10 for station 40WZ01
%
% See also rio_init, rio_checkdeployment
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function Cnf = rio_dbdaql( Cnf )

switch Cnf.deployment
    case { 'VITO', 'IRCEL' }        
        %-- Dump erratic pm10 data in 40WZ01 station between
        %-- 1/06/2004 and 31/07/2005 and 20/03/2006 and 10/08/2006
        st_40WZ01_i = find(strcmp( Cnf.st_id, '40WZ01'));  
        if strcmp( Cnf.pol_xx, 'pm10' )
            st_dump_i = find(Cnf.xx_val(:,1) == st_40WZ01_i);
            d_dump_i_l_1 = find(Cnf.xx_date >= datenum(2004,06,01));
            d_dump_i_h_1 = find(Cnf.xx_date <= datenum(2005,07,31));
            d_dump_i_1 = intersect(d_dump_i_l_1, d_dump_i_h_1);
            
            d_dump_i_l_2 = find(Cnf.xx_date >= datenum(2006,03,20));
            d_dump_i_h_2 = find(Cnf.xx_date <= datenum(2006,08,10));
            d_dump_i_2 = intersect(d_dump_i_l_2, d_dump_i_h_2);
            
            d_dump_i = union(d_dump_i_1, d_dump_i_2);
            
            dump_i = intersect(st_dump_i, d_dump_i);
            
            Cnf.xx_date(dump_i) = [];
            Cnf.xx_val(dump_i,:) = [];
        end
        
        %-- For o3, select summer data only
        if strcmp(Cnf.pol, 'o3s')            
            dv = datevec( Cnf.xx_date );                          %-- date vector yyyy, mm, dd
            winter_i = find(dv(:,2) < 4 | dv(:,2) > 9);
            Cnf.xx_date(winter_i)  = [];
            Cnf.xx_val(winter_i,:) = [];
        end
        

    otherwise 
        % do nothing for now...
end


end