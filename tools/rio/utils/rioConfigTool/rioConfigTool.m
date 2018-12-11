function varargout = rioConfigTool(varargin)
% RIOCONFIGTOOL MATLAB code for rioConfigTool.fig
%      RIOCONFIGTOOL, by itself, creates a new RIOCONFIGTOOL or raises the existing
%      singleton*.
%
%      H = RIOCONFIGTOOL returns the handle to a new RIOCONFIGTOOL or the handle to
%      the existing singleton*.
%
%      RIOCONFIGTOOL('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in RIOCONFIGTOOL.M with the given input arguments.
%
%      RIOCONFIGTOOL('Property','Value',...) creates a new RIOCONFIGTOOL or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before rioConfigTool_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to rioConfigTool_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
% %      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help rioConfigTool

% Last Modified by GUIDE v2.5 19-Nov-2013 13:46:04

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @rioConfigTool_OpeningFcn, ...
                   'gui_OutputFcn',  @rioConfigTool_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before rioConfigTool is made visible.
function rioConfigTool_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to rioConfigTool (see VARARGIN)

% Choose default command line output for rioConfigTool
handles.output = hObject;

% set some defaults
set( handles.basePathEdit, 'String', pwd );
fname = fullfile( pwd, 'rio_setup.xml');
if exist( fname, 'file' )
  set( handles.configFileNameEdit, 'String', fname );
  cnfList = getConfigurations( fname );
  polList = getConfigurationPollutants( fname, cnfList{1} );
  drvList = getConfigurationPollutantDrivers( fname, cnfList{1}, polList{1} );
  set( handles.configMenu, 'String', cnfList );
  set( handles.polMenu, 'String', polList );
  set( handles.drvMenu, 'String', drvList );
end

% default is a 2nd degree trend on the averages, and a 1st degree trend on
% the stddev values...
set( handles.avgTrendFcnPopupMenu, 'Value', 2 );
set( handles.stdTrendFcnPopupMenu, 'Value', 1 );

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes rioConfigTool wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = rioConfigTool_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on selection change in polMenu.
function polMenu_Callback(hObject, eventdata, handles)
% hObject    handle to polMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns polMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from polMenu
fname = get( handles.configFileNameEdit, 'String' );

cnfCont = cellstr(get(handles.configMenu, 'String'));
cnfName = cnfCont(get(handles.configMenu, 'Value'));

polCont = cellstr(get(hObject,'String'));
polName = polCont{get(hObject,'Value')};

drvList = getConfigurationPollutantDrivers( fname, cnfName, polName );

set( handles.drvMenu, 'String', drvList );

guidata(hObject,handles);


% --- Executes during object creation, after setting all properties.
function polMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to polMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in configMenu.
function configMenu_Callback(hObject, eventdata, handles)
% hObject    handle to configMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns configMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from configMenu
fname = get( handles.configFileNameEdit, 'String' );

contents = cellstr(get(hObject,'String'));
cnfName  = contents{get(hObject,'Value')};

polList = getConfigurationPollutants( fname, cnfName );
drvList = getConfigurationPollutantDrivers( fname, cnfName, polList{1} );
set( handles.polMenu, 'String', polList );
set( handles.drvMenu, 'String', drvList );

guidata(hObject,handles);


% --- Executes during object creation, after setting all properties.
function configMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to configMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in aggMenu.
function aggMenu_Callback(hObject, eventdata, handles)
% hObject    handle to aggMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns aggMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from aggMenu


% --- Executes during object creation, after setting all properties.
function aggMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to aggMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function configFileNameEdit_Callback(hObject, eventdata, handles)
% hObject    handle to configFileNameEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes during object creation, after setting all properties.
function configFileNameEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to configFileNameEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in setConfigFileButton.
function setConfigFileButton_Callback(hObject, eventdata, handles)
% hObject    handle to setConfigFileButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
[filename, pathname] = uigetfile({ '*.xml', 'XML Config files (*.xml)';...
  '*.*', 'All files...'}, 'Load config file...', 'MultiSelect', 'off' );
fname = fullfile(pathname,filename);
if ~( isequal(filename,0) || isequal(pathname,0) )  
  set( handles.configFileNameEdit, 'String', fname );
end

cnfList = getConfigurations( fname );
polList = getConfigurationPollutants( fname, cnfList{1} );
drvList = getConfigurationPollutantDrivers( fname, cnfList{1}, polList{1} );
set( handles.configMenu, 'String', cnfList );
set( handles.polMenu, 'String', polList );
set( handles.drvMenu, 'String', drvList );

guidata(hObject,handles);

% --- Executes on selection change in gridMenu.
function gridMenu_Callback(hObject, eventdata, handles)
% hObject    handle to gridMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns gridMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from gridMenu


% --- Executes during object creation, after setting all properties.
function gridMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to gridMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in drvMenu.
function drvMenu_Callback(hObject, eventdata, handles)
% hObject    handle to drvMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns drvMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from drvMenu


% --- Executes during object creation, after setting all properties.
function drvMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to drvMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in weekMenu.
function weekMenu_Callback(hObject, eventdata, handles)
% hObject    handle to weekMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns weekMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from weekMenu


% --- Executes during object creation, after setting all properties.
function weekMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to weekMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function startDateEdit_Callback(hObject, eventdata, handles)
% hObject    handle to startDateEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of startDateEdit as text
%        str2double(get(hObject,'String')) returns contents of startDateEdit as a double


% --- Executes during object creation, after setting all properties.
function startDateEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to startDateEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function stopDateEdit_Callback(hObject, eventdata, handles)
% hObject    handle to stopDateEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of stopDateEdit as text
%        str2double(get(hObject,'String')) returns contents of stopDateEdit as a double


% --- Executes during object creation, after setting all properties.
function stopDateEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to stopDateEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in calcStatButton.
function calcStatButton_Callback(hObject, eventdata, handles)
% hObject    handle to calcStatButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

s = readConfig( handles );
fprintf( 'Config : ' );
disp(s);

% configure rio
cnf = rio_setup( s.setup_file, s.setup_conf, s.pollutant, ...
  s.agg_time, s.gis_type, s.grid_type  );

cnf.deployment = '';
cnf = rio_loadstationinfo( cnf );
cnf = rio_loaddb( cnf );
cnf = rio_dbdaql( cnf );

% apply the log transformation outside of the load/create database routines
% also best do this after the data quality checks...
if cnf.Option.logtrans
  fprintf( 'Applying log transformation to measurement database...\n' );
  cnf.xx_val(:,2:end) = log( 1. + cnf.xx_val(:,2:end) );
end

%get additional options
cutoff = str2double( get( handles.statOptCutoffEdit, 'String' ) );

% Trend determination options, see help rio_gettrend for more info
opts = struct( 'weekpart',    s.weekpart, ...         % part of the week
  'time_window', s.window, ...         % window to compute statistics
  'cutoff',      cutoff, ...           % values above the cutoff concentration are ignored in the statistics
  'overwrite',  'ask' );               % overwrite the trend parameters

%-- we determine the trend
[ xx_avg, xx_std ] = rio_calcstats( cnf, opts );



% --- Executes on button press in trendButton.
function trendButton_Callback(hObject, eventdata, handles)
% hObject    handle to trendButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

s = readConfig( handles );
fprintf( 'Config : ' );
disp(s);

% get fit mode
switch get( handles.fitModePopupMenu, 'Value' );
  case 1,
    fitmode = 'polyfit';
  case 2,
    fitmode = 'nlinfit';
  otherwise
    fitmode = 'polyfit';
    warning('unknown fitmode, using default')    ;
end

% get trend function order
tmp = get( handles.avgTrendFcnPopupMenu, 'String' );
tmp = tmp{ get( handles.avgTrendFcnPopupMenu, 'Value' ) };
switch tmp
  case 'poly1', avg_order = 1;
  case 'poly2', avg_order = 2;
  otherwise
    error( 'unknown avg trend order' );
end    
tmp = get( handles.stdTrendFcnPopupMenu, 'String' );
tmp = tmp{ get( handles.stdTrendFcnPopupMenu, 'Value' ) };
switch tmp
  case 'poly1', std_order = 1;
  case 'poly2', std_order = 2;
  otherwise
    error( 'unknown avg trend order' );
end


% plot ranges (in std. concentration units (log trans is automatic)
avg_yrange = [ str2double( get( handles.avgYRangeMinEdit, 'String') ) ...
  str2double( get( handles.avgYRangeMaxEdit, 'String') ) ];
std_yrange = [ str2double( get( handles.stdYRangeMinEdit, 'String') ) ...
  str2double( get( handles.stdYRangeMaxEdit, 'String') ) ];

cnf = rio_setup( s.setup_file, s.setup_conf, s.pollutant, s.agg_time, s.gis_type, s.grid_type  );
cnf.deployment = '';
cnf.ipol_mode  = 'RIO';
cnf = rio_loadstationinfo( cnf );
cnf = rio_loaddb( cnf );
cnf = rio_dbdaql( cnf );
% apply the log transformation outside of the load/create database routines
% also best do this after the data quality checks...
if cnf.Option.logtrans
  fprintf( 'Applying log transformation to measurement database...\n' );
  cnf.xx_val(:,2:end) = log( 1. + cnf.xx_val(:,2:end) );
end

% get station ignore list...
%handles.stationIgnoreListEdit
str = get( handles.stationIgnoreListEdit, 'String' );
st_list = [];
if ~isempty( str )
  x = textscan( str, '%s', 'delimiter', ',' );
  x = x{1};
  
  for i=1:length(x)
    idx = find( strcmp( cnf.st_id, x{i} ) );
    if isscalar(idx)
      st_list = [ st_list cnf.st_info(idx,1) ];
      fprintf( 'Adding station %s (%d) to ignore list.\n', x{i}, cnf.st_info(idx,1) )
    end
  end
end

% Trend determination options, see help rio_gettrend for more info
trend_options = struct( ...
    'calcstats',       false, ...       % re-compute statistics, or load ?
    'weekpart',        s.weekpart, ...  % part of the week 
    'fitmode',         fitmode, ...     % polyfit / nlinfit
    'ignore_stations', st_list, ...     % list of stations not to include in trend fitting
    'order_avg',       avg_order, ...
    'order_std',       std_order, ...
    'make_plot',       get( handles.makePlotCheckBox,  'Value' ), ... % make the plot
    'save_plot',       get( handles.savePlotCheckBox,  'Value' ), ... % save the plot
    'show_plot',       get( handles.showPlotCheckBox,  'Value' ), ... % show the plot
    'show_stats',      get( handles.showStatsCheckBox, 'Value' ), ... % some regression statistics
    'plot_labels',     get( handles.plotLabelsCheckBox, 'Value' ), ... % plot the station labels as well
    'label_dx',        str2double( get( handles.dxLabelEdit, 'String' ) ), ... % label horizontal shift
    'label_dy',        str2double( get( handles.dyLabelEdit, 'String' ) ),  ...% label vertical shift
    'fancy_plot',      get( handles.fancyPlotCheckBox, 'Value'), ...    % different colour per station type
    'fancy_legend',    get( handles.fancyLegendCheckBox, 'Value' ), ... % legend for fancy plot
    'plot_avg_yrange', avg_yrange, ...    % fix y range for avg trend plot
    'plot_std_yrange', std_yrange, ...    % fix y range for std trend plot
    'overwrite',       'ask' );           % overwrite the trend parameters

%-- we determine the trend
rio_gettrend( cnf, trend_options );



% --- Executes on button press in spcorrButton.
function spcorrButton_Callback(hObject, eventdata, handles)
% hObject    handle to spcorrButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

s = readConfig( handles );
fprintf( 'Config : ' );
disp(s);

% configure rio
cnf = rio_setup( s.setup_file, s.setup_conf, s.pollutant, s.agg_time, s.gis_type, s.grid_type  );
cnf.deployment = '';
cnf.ipol_mode  = 'RIO';
cnf = rio_loadstationinfo( cnf );
cnf = rio_loaddb( cnf );
cnf = rio_dbdaql( cnf );
% apply the log transformation outside of the load/create database routines
% also best do this after the data quality checks...
if cnf.Option.logtrans
  fprintf( 'Applying log transformation to measurement database...\n' );
  cnf.xx_val(:,2:end) = log( 1. + cnf.xx_val(:,2:end) );
end
%

switch get( handles.corrModelPopupMenu, 'Value' );
  case 1,
    corrmodel = 'exp';
  case 2,
    corrmodel = 'expm';
  otherwise
    corrmodel = 'exp';
    warning('unknown correlogram model, using default')    ;
end

% Setup the option structure
sp_options = struct( ...
    'time_window',  s.window, ...  % use this time window
    'make_plot',    get( handles.makeSpCorrPlotCheckBox, 'Value'), ... % make a nice plot
    'short',        get( handles.useShortRangeCheckBox, 'Value' ), ...     % use short range correlation model as well
    'short_range',  str2double( get( handles.shortRangeEdit, 'String' ) ), ... % the range for it
    'detrend',      get( handles.detrendCheckBox, 'Value' ), ...          % detrend the data
    'profile',      get( handles.makeProfileCheckBox, 'Value'), ...       % fit the profile histogram    
    'prof_binsize', str2double( get( handles.binsizeEdit, 'String') ), ...    % binsize in km for profile histogram
    'corr_model', corrmodel, ...    % spatial correlation model (exp, expm)
    'overwrite', 'ask' );

%-- we determine the spatial correlation model
[ p, p_short ] = rio_calcspcorr( cnf, sp_options );


function basePathEdit_Callback(hObject, eventdata, handles)
% hObject    handle to basePathEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of basePathEdit as text
%        str2double(get(hObject,'String')) returns contents of basePathEdit as a double


% --- Executes during object creation, after setting all properties.
function basePathEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to basePathEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in setBasePathButton.
function setBasePathButton_Callback(hObject, eventdata, handles)
% hObject    handle to setBasePathButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
pathname = uigetdir( '.', 'Select working directory...');
if ~isequal(pathname,0) 
  set( handles.basePathEdit, 'String', pathname );
end
cd(pathname);
fprintf( 'Changing to working directory %s...\n', pwd );


% --- Executes on button press in buildDBButton.
function buildDBButton_Callback(hObject, eventdata, handles)
% hObject    handle to buildDBButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

s = readConfig( handles );
fprintf( 'Config : ' );
disp(s);

cnf = rio_setup( s.setup_file, s.setup_conf, s.pollutant, s.agg_time, s.gis_type, s.grid_type  );
cnf.deployment = 'IRCEL';
cnf = rio_loadstationinfo( cnf );
rio_createdb( cnf )


function s = readConfig( h )

s = struct();
s.setup_file = strtrim(get( h.configFileNameEdit, 'String' ));

tt = get( h.configMenu, 'String' );
s.setup_conf = strtrim(tt{get( h.configMenu, 'Value' )});

tt = get( h.polMenu, 'String' );
s.pollutant  = strtrim(tt{get( h.polMenu, 'Value' )});

tt = get( h.aggMenu, 'String' );
s.agg_time   = strtrim(tt{get( h.aggMenu, 'Value' )});

tt = get( h.gridMenu, 'String' );
s.grid_type   = strtrim(tt{get( h.gridMenu, 'Value' )});

tt = get( h.drvMenu, 'String' );
s.gis_type  = strtrim(tt{get( h.drvMenu, 'Value' )});

tt = get( h.weekMenu, 'String' );
s.weekpart  = strtrim(tt{get( h.weekMenu, 'Value' )});

try
  t_start = floor(datenum( strtrim(get( h.startDateEdit, 'String' )), 'yyyy-mm-dd' ));
  t_stop  = floor(datenum( strtrim(get( h.stopDateEdit, 'String' )), 'yyyy-mm-dd' ))+datenum(0,0,0,23,59,59);
  s.window = [ t_start t_stop ];
catch ex
  errordlg( 'Please use YYYY-MM-DD format for date...','Error in date','modal');
  rethrow(ex);
end

function statOptCutoffEdit_Callback(hObject, eventdata, handles)
% hObject    handle to statOptCutoffEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of statOptCutoffEdit as text
%        str2double(get(hObject,'String')) returns contents of statOptCutoffEdit as a double


% --- Executes during object creation, after setting all properties.
function statOptCutoffEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to statOptCutoffEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in fitModePopupMenu.
function fitModePopupMenu_Callback(hObject, eventdata, handles)
% hObject    handle to fitModePopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns fitModePopupMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from fitModePopupMenu


% --- Executes during object creation, after setting all properties.
function fitModePopupMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to fitModePopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function stationIgnoreListEdit_Callback(hObject, eventdata, handles)
% hObject    handle to stationIgnoreListEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of stationIgnoreListEdit as text
%        str2double(get(hObject,'String')) returns contents of stationIgnoreListEdit as a double


% --- Executes during object creation, after setting all properties.
function stationIgnoreListEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to stationIgnoreListEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in makePlotCheckBox.
function makePlotCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to makePlotCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of makePlotCheckBox


% --- Executes on button press in savePlotCheckBox.
function savePlotCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to savePlotCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of savePlotCheckBox


% --- Executes on button press in showPlotCheckBox.
function showPlotCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to showPlotCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of showPlotCheckBox


% --- Executes on button press in showStatsCheckBox.
function showStatsCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to showStatsCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of showStatsCheckBox


% --- Executes on button press in plotLabelsCheckBox.
function plotLabelsCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to plotLabelsCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of plotLabelsCheckBox


% --- Executes on button press in fancyPlotCheckBox.
function fancyPlotCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to fancyPlotCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of fancyPlotCheckBox


% --- Executes on button press in fancyLegendCheckBox.
function fancyLegendCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to fancyLegendCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of fancyLegendCheckBox



function dxLabelEdit_Callback(hObject, eventdata, handles)
% hObject    handle to dxLabelEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of dxLabelEdit as text
%        str2double(get(hObject,'String')) returns contents of dxLabelEdit as a double


% --- Executes during object creation, after setting all properties.
function dxLabelEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to dxLabelEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function dyLabelEdit_Callback(hObject, eventdata, handles)
% hObject    handle to dyLabelEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of dyLabelEdit as text
%        str2double(get(hObject,'String')) returns contents of dyLabelEdit as a double


% --- Executes during object creation, after setting all properties.
function dyLabelEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to dyLabelEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function avgYRangeMinEdit_Callback(hObject, eventdata, handles)
% hObject    handle to avgYRangeMinEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of avgYRangeMinEdit as text
%        str2double(get(hObject,'String')) returns contents of avgYRangeMinEdit as a double


% --- Executes during object creation, after setting all properties.
function avgYRangeMinEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to avgYRangeMinEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function avgYRangeMaxEdit_Callback(hObject, eventdata, handles)
% hObject    handle to avgYRangeMaxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of avgYRangeMaxEdit as text
%        str2double(get(hObject,'String')) returns contents of avgYRangeMaxEdit as a double


% --- Executes during object creation, after setting all properties.
function avgYRangeMaxEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to avgYRangeMaxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function stdYRangeMinEdit_Callback(hObject, eventdata, handles)
% hObject    handle to stdYRangeMinEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of stdYRangeMinEdit as text
%        str2double(get(hObject,'String')) returns contents of stdYRangeMinEdit as a double


% --- Executes during object creation, after setting all properties.
function stdYRangeMinEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to stdYRangeMinEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function stdYRangeMaxEdit_Callback(hObject, eventdata, handles)
% hObject    handle to stdYRangeMaxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of stdYRangeMaxEdit as text
%        str2double(get(hObject,'String')) returns contents of stdYRangeMaxEdit as a double


% --- Executes during object creation, after setting all properties.
function stdYRangeMaxEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to stdYRangeMaxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in makeSpCorrPlotCheckBox.
function makeSpCorrPlotCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to makeSpCorrPlotCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of makeSpCorrPlotCheckBox


% --- Executes on button press in useShortRangeCheckBox.
function useShortRangeCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to useShortRangeCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of useShortRangeCheckBox


% --- Executes on button press in detrendCheckBox.
function detrendCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to detrendCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of detrendCheckBox



function shortRangeEdit_Callback(hObject, eventdata, handles)
% hObject    handle to shortRangeEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of shortRangeEdit as text
%        str2double(get(hObject,'String')) returns contents of shortRangeEdit as a double


% --- Executes during object creation, after setting all properties.
function shortRangeEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to shortRangeEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in makeProfileCheckBox.
function makeProfileCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to makeProfileCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of makeProfileCheckBox



function binsizeEdit_Callback(hObject, eventdata, handles)
% hObject    handle to binsizeEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of binsizeEdit as text
%        str2double(get(hObject,'String')) returns contents of binsizeEdit as a double


% --- Executes during object creation, after setting all properties.
function binsizeEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to binsizeEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in corrModelPopupMenu.
function corrModelPopupMenu_Callback(hObject, eventdata, handles)
% hObject    handle to corrModelPopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns corrModelPopupMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from corrModelPopupMenu


% --- Executes during object creation, after setting all properties.
function corrModelPopupMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to corrModelPopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in avgTrendFcnPopupMenu.
function avgTrendFcnPopupMenu_Callback(hObject, eventdata, handles)
% hObject    handle to avgTrendFcnPopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

warndlg( 'Make sure you update the xml configuration file to match the trend function selection before running the spatial correlation calculation or before interpolating !!', 'Trend function change warning','modal');

% Hints: contents = cellstr(get(hObject,'String')) returns avgTrendFcnPopupMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from avgTrendFcnPopupMenu


% --- Executes during object creation, after setting all properties.
function avgTrendFcnPopupMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to avgTrendFcnPopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in stdTrendFcnPopupMenu.
function stdTrendFcnPopupMenu_Callback(hObject, eventdata, handles)
% hObject    handle to stdTrendFcnPopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
warndlg( 'Make sure you update the xml configuration file to match the trend function selection before running the spatial correlation calculation or before interpolating !!', 'Trend function change warning','modal');
% Hints: contents = cellstr(get(hObject,'String')) returns stdTrendFcnPopupMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from stdTrendFcnPopupMenu


% --- Executes during object creation, after setting all properties.
function stdTrendFcnPopupMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to stdTrendFcnPopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function exportPathEdit_Callback(hObject, eventdata, handles)
% hObject    handle to exportPathEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of exportPathEdit as text
%        str2double(get(hObject,'String')) returns contents of exportPathEdit as a double


% --- Executes during object creation, after setting all properties.
function exportPathEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to exportPathEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in exportPathSetButton.
function exportPathSetButton_Callback(hObject, eventdata, handles)
% hObject    handle to exportPathSetButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
pathname = uigetdir( '.', 'Set export directory...');
if ~isequal(pathname,0) 
  set( handles.exportPathEdit, 'String', pathname );
end
guidata(hObject,handles);

% --- Executes on button press in exportButton.
function exportButton_Callback(hObject, eventdata, handles)
% hObject    handle to exportButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

s       = readConfig( handles );
outbase = get( handles.exportPathEdit, 'String' );
base    = get( handles.basePathEdit, 'String' );
export_grid     = true;
export_stations = true;

% create output folder structure
trend_dir = fullfile( outbase, 'param', s.setup_conf, s.pollutant, 'trend', '' );
spcor_dir = fullfile( outbase, 'param', s.setup_conf, s.pollutant, 'spatial_corr', '' );
statp_dir = fullfile( outbase, 'param', s.setup_conf, s.pollutant, 'stat_param', '' );
stati_dir = fullfile( outbase, 'stations', s.setup_conf, s.pollutant, '' );
landu_dir = fullfile( outbase, 'drivers',  s.setup_conf, s.pollutant , '' );
if ~exist( trend_dir, 'file' ), mkdir( trend_dir); end;
if ~exist( spcor_dir, 'file' ), mkdir( spcor_dir); end;
if ~exist( statp_dir, 'file' ), mkdir( statp_dir); end;
if ~exist( stati_dir, 'file' ), mkdir( stati_dir); end;
if ~exist( landu_dir, 'file' ), mkdir( landu_dir); end;

% export grid
if export_grid
    fprintf( 'Exporting grid...\n' );
    grid_file = fullfile( base, 'drivers', s.setup_conf, s.pollutant, ...
      sprintf( '%s_grid_%s.txt', s.gis_type, s.grid_type ) );    
    copyfile( grid_file, landu_dir );
end

% export stations
if export_stations
    fprintf( 'Exporting stations...\n' );
    station_file = fullfile( base, 'stations', s.setup_conf, s.pollutant, ...
      sprintf( '%s_stations_info_GIS_%s.txt', s.pollutant, s.gis_type ) );    
    copyfile( station_file, stati_dir );
end
   
% export the trend functions
fprintf( 'Exporting rio parameters functions...\n' );
for param = { 'trend', 'spatial_corr', 'stat_param' }    
    
    if strcmp( param, 'spatial_corr' )        
        range_list_1 = {'long', 'short'};
        range_list_2 = {''};
    elseif strcmp( param, 'trend')        
        range_list_1 = {'week', 'weekend', 'all'}; 
        range_list_2 = {'avg', 'std', 'avg_err', 'std_err' };
    elseif strcmp( param, 'stat_param')        
        range_list_1 = {'week', 'weekend', 'all'};
        range_list_2 = {'avg' , 'std'};
    end
            
    for range_1 = range_list_1
        for range_2 = range_list_2
            
            fprintf( 'Exporting %s : %s %s\n', char(param), char(range_1), char(range_2) );
            
            switch( char( param ) )
                case 'trend'                  
                  infile = fullfile( base, 'param', s.setup_conf, s.pollutant, char(param), ...
                    sprintf( '%s_trend_%s_%s_%s_agg_time-%s.mat',  char(range_2), ...
                    s.pollutant, s.gis_type, char(range_1), s.agg_time ) );
                  outfile = fullfile( trend_dir, ...
                    sprintf( '%s_trend_%s_%s_%s_agg_time-%s.txt',  char(range_2), ...
                    s.pollutant, s.gis_type, char(range_1), s.agg_time ) );
                    
              case 'spatial_corr'                
                infile = fullfile( base, 'param', s.setup_conf, s.pollutant, 'spatial_corr', ...
                  sprintf( 'p_%s_%s_%s_agg_time-%s.mat',  char(range_1), ...
                  s.pollutant, s.gis_type, s.agg_time ) );
                outfile = fullfile( spcor_dir, ...
                  sprintf( 'p_%s_%s_%s_agg_time-%s.txt',  char(range_1), ...
                  s.pollutant, s.gis_type, s.agg_time ) );
                
                case 'stat_param'                 
                  infile = fullfile( base, 'param', s.setup_conf, s.pollutant, 'stat_param', ...
                    sprintf( '%s_%s_%s_agg_time-%s.mat', char(range_2), ...
                    s.pollutant, char(range_1), s.agg_time ) );                  
                  outfile = fullfile( statp_dir, ...
                    sprintf( '%s_%s_%s_agg_time-%s.txt', char(range_2), ...
                    s.pollutant, char(range_1), s.agg_time ) );
            end
            
            % load the file
            p_tmp = load( infile );
            
            % select the right variable
            switch( char( param ) )
                case 'trend'
                    if strcmp(range_2, 'avg')
                        p = p_tmp.p_avg;
                    elseif strcmp(range_2, 'std')
                        p = p_tmp.p_std;
                    elseif strcmp(range_2, 'avg_err')
                        p = p_tmp.p_avg_err;
                    elseif strcmp(range_2, 'std_err')
                        p = p_tmp.p_std_err;                        
                    end
                case 'spatial_corr'
                    if strcmp(range_1, 'long')
                        p = p_tmp.p;
                    elseif strcmp(range_1, 'short')
                        p = p_tmp.p_short;
                    end       
                case 'stat_param'
                    if strcmp(range_2, 'avg')
                        p = p_tmp.xx_avg;
                    elseif strcmp(range_2, 'std')
                        p = p_tmp.xx_std;
                    end
            end
            
            % export to ascii
            fprintf( 'writing %s -> %s\n', infile, outfile );
            save( outfile, 'p', '-ascii');
            
            clear p p_tmp;
        end
    end
end
