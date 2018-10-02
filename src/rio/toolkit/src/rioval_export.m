%RIOVAL_EXPORT Exports the validation timeseries to hdf5 file
%
% rioval_export( fname, cnf, opts, xx_dates, v )
% rioval_export( matfile, h5file )
%
% RIO (c) VITO/IRCEL 2004-2013
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rioval_export( varargin )

%% Init
if nargin == 2
  tt = load( varargin{1} );
  try
    cnf      = tt.cnf;
    opts     = tt.opts;
    xx_dates = tt.xx_dates;
    v        = tt.xx_val;
    fname    = varargin{2};
    clear tt;
  catch ME
    error( 'Matlab file was not generated with rioval...' );
  end  
elseif nargin == 5
  fname = varargin{1};
  cnf   = varargin{2};
  opts  = varargin{3};
  xx_dates = varargin{4};
  v        = varargin{5}; 
else
  error( 'Wrong number of arguments...' );
end

%% Here we go
dv = datevec( xx_dates );
sz = size(xx_dates);
h5create( fname, '/time/year',  sz ); h5write( fname, '/time/year',  dv(:,1) );
h5create( fname, '/time/month', sz ); h5write( fname, '/time/month', dv(:,2) );
h5create( fname, '/time/day',   sz ); h5write( fname, '/time/day',   dv(:,3) );
h5create( fname, '/time/hour',  sz ); h5write( fname, '/time/hour',  dv(:,4) );
h5create( fname, '/time/matlab',sz ); h5write( fname, '/time/matlab',xx_dates );
for st_id=1:cnf.nr_st
  if ~v(st_id).have_station, continue; end;

  h5create( fname, sprintf( '/validation/%s/mod', upper( v(st_id).name ) ),  sz );
  h5write( fname,  sprintf( '/validation/%s/mod', upper( v(st_id).name ) ),  v(st_id).xx_mod );
  
  h5create( fname, sprintf( '/validation/%s/obs', upper( v(st_id).name ) ),  sz );
  h5write( fname,  sprintf( '/validation/%s/obs', upper( v(st_id).name ) ),  v(st_id).xx_obs );
  
  
  h5writeatt( fname, sprintf( '/validation/%s', upper( v(st_id).name ) ), 'type',  int16(v(st_id).type) );
  h5writeatt( fname, sprintf( '/validation/%s', upper( v(st_id).name ) ), 'indic', v(st_id).indic );  
  h5writeatt( fname, sprintf( '/validation/%s', upper( v(st_id).name ) ), 'x',     v(st_id).x );
  h5writeatt( fname, sprintf( '/validation/%s', upper( v(st_id).name ) ), 'y',     v(st_id).y );
  
  
end
ver = rio_version;
% write some attributes
h5writeatt( fname, '/', 'MissingValue', cnf.missing_value );
h5writeatt( fname, '/', 'Pollutant',    cnf.pol );
h5writeatt( fname, '/', 'AggregationTime', cnf.at_lb );
h5writeatt( fname, '/', 'Proxy',           cnf.gis_type );
h5writeatt( fname, '/', 'InterpolationMode', cnf.ipol_mode );
h5writeatt( fname, '/', 'Deployment',        cnf.deployment );
h5writeatt( fname, '/', 'librio_minor', ver.minor );
h5writeatt( fname, '/', 'librio_major', ver.major );
h5writeatt( fname, '/', 'SetupFile',       opts.setup_file );
h5writeatt( fname, '/', 'Configuration',   opts.conf );
