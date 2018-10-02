%RIO_VALIDATE
% Calculates some validation statistics using a leaving-one-out approach
% for the station defined by the station index argument. The interpolation 
% model is run for the dates requested and each time, we interpolate
% exactly to the station location, making use of the landuse parameter of
% the station itself. 
% 
% [ stats, model, obser ] = rio_validate( cnf, st_i, dates )
%
% Returned are a structure containing some validation info, a timeseries
% with the model and the observation. The validation structure contains
%  - stats.bias  : model bias ( model-observ)
%  - stats.rmse  : root mean square error
%  - stats.r     : correlation coefficient (not squared)
%  - stats.acc   : anomaly correlation, climatology is mean of observation
%
% Note: this routine is only implemented for the RIO and the OK
%       interpolation
%
% ChangeLog:
%  - 2015/06 : allowed for more than one st_i --> to be used in MC
%              validation. if more than one st_i if requested, we return
%              a cell array with stats& model & observation values
%              corresponding to the st_i given... 
%
% 
% See also rio_init, rio_updatepars, rio_covmat, rio_detrend, rio_krige,
%          rio_addtrend
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ stats, model, obser ] = rio_validate( cnf, st_i, dates )

if numel( st_i ) < 1
    error( 'rio_validate', 'please provide at least one station to leave out' );
end

fprintf( 'rio_validate:: starting validation for station(s) (leaving out) : \n' ); 
fprintf( 'rio_validate:: %s\n', cnf.st_id{ st_i } );
fprintf( 'rio_validate:: processing %d dates\n', length(dates) );

% TODO : adjust to multiple st_i !!
stats = struct( 'rmse', 0, 'bias', 0 );


model = zeros( length(dates), numel(st_i) );
obser = zeros( length(dates), numel(st_i) );    


for k=1:length(dates)
    cnf = rio_updatepars( cnf, dates(k) );
    
    [st_info_tmp, xx_data] = rio_dblookup( cnf, dates(k) );   
    
    % now get rid of the data of the current station...        
    st_x     = cnf.st_info( st_i, 2 )./1000; % still need to convert to km...
    st_y     = cnf.st_info( st_i, 3 )./1000;    
    st_indic = cnf.st_indic( st_i, : );
    
    % now get rid of the current station to interpolate
    % BM[2015.06] : allow multiple selected stations
    % idx_current = find( st_info_tmp(:,1) == st_i );    
    [ ~, idx_current, idx_st ] = intersect( st_info_tmp(:,1), st_i );
    
    xx_obs = nan(1,numel(st_i));
    if ~isempty( idx_current )
        xx_obs(idx_st) = xx_data( idx_current, 2 );
        st_info_tmp( idx_current, : ) = [];
        xx_data( idx_current, : )     = [];
    end
    
    if ~isempty(xx_data)
      
      % now interpolate from here to the location of the missing station
      switch cnf.ipol_mode
        case 'RIO'
          xx_detr = rio_detrend( cnf, st_info_tmp, xx_data );
          C_inv   = rio_covmat( cnf, st_info_tmp );
          
          % this can be much more efficient...
          xx = nan(1,numel(st_i));
          for i=1:numel(st_i)
              [ val, err ] = rio_krige( cnf, C_inv, st_info_tmp, xx_detr, st_x(i), st_y(i) );
              xx(i) = rio_addtrend( cnf, val, err, st_indic(i) );
          end
          
        case 'OrdKrig'
          C_inv   = rio_covmat( cnf, st_info_tmp );
          for i=1:numel(st_i)
              xx(i) = rio_krige( cnf, C_inv, st_info, xx_data, st_x(i), st_y(i) );
          end
          
        otherwise
          error( 'rio_validate:: mode %s not implemented here', cnf.ipol_mode );
          return;
      end
      
      if cnf.Option.logtrans
        % if we are using the log transformation, transform back to normal
        % concentrations...
        xx     = exp(xx) - 1;
        xx_obs = exp(xx_obs) - 1;
      end
      model( k,:) = xx;     % the model
      obser( k,:) = xx_obs; % the observation
      
    else
      model(k,:) = -999;
      obser(k,:) = xx_obs;
    end
        
end
% model - observation ( bias )
stats.bias    = nanmean( model - obser );
stats.rmse    = sqrt( nanmean( ( model - obser ).^2 ) );
stats.obs_avg = nanmean(obser); % the climate : from the observations
for i=1:numel(st_i)
    C = nancov( model(:,i), obser(:,i) );
    stats.r(i) = C(1,2)/sqrt(C(1,1)*C(2,2));    
end
stats.r2 = (stats.r).^2;

end
