function varargout = OptimizeGUI(varargin)
% OPTIMIZEGUI M-file for OptimizeGUI.fig
%      OPTIMIZEGUI, by itself, creates a new OPTIMIZEGUI or raises the existing
%      singleton*.
%
%      H = OPTIMIZEGUI returns the handle to a new OPTIMIZEGUI or the handle to
%      the existing singleton*.
%
%      OPTIMIZEGUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in OPTIMIZEGUI.M with the given input arguments.
%
%      OPTIMIZEGUI('Property','Value',...) creates a new OPTIMIZEGUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before OptimizeGUI_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to OptimizeGUI_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help OptimizeGUI

% Last Modified by GUIDE v2.5 05-Jan-2017 11:25:54

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @OptimizeGUI_OpeningFcn, ...
                   'gui_OutputFcn',  @OptimizeGUI_OutputFcn, ...
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


% --- Executes just before OptimizeGUI is made visible.
function OptimizeGUI_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to OptimizeGUI (see VARARGIN)


set(0,'defaultUicontrolFontName', 'Calibri');
set(0,'defaultUitableFontName', 'Calibri');
set(0,'defaultAxesFontName', 'Calibri');
set(0,'defaultTextFontName', 'Calibri');
set(0,'defaultUipanelFontName', 'Calibri');

% Choose default command line output for OptimizeGUI
handles.output = hObject;
cla(handles.axes1,'reset')
cla(handles.axes2,'reset')

% Load configuration and select deployment
cf = load( 'OptimizeGUIConfig.mat' );
id = listdlg( 'Name', 'OptimizeGUI', 'PromptString', 'Select deployment', ... 
    'SelectionMode', 'single', 'ListSize', [ 150, 120 ], ...
    'ListString', {cf.dpl.name} );
handles.dpl     = cf.dpl(id);

% Select pollutant in deployment
id = listdlg( 'Name', 'OptimizeGUI', 'PromptString', 'Select pollutant', ... 
    'SelectionMode', 'single', 'ListSize', [ 150, 120 ], ...
    'ListString', handles.dpl.pollutant_options );

handles.pol_xx          = handles.dpl.pollutant_options{id};
handles.rio_class_names = handles.dpl.class_names;
handles.rio_class_agg   = handles.dpl.class_aggregation;

% Select pollutant & aggregation time to do minim
ignore_list       = [];

% Set some configuration flags
handles.labels     = 'ID';
handles.gis_type   = 'CorineID';
handles.fit_order  = 2; % only for 1D trend...
handles.norm       = 'MSE';
handles.n_dim           = 1; % number of beta parameters ?
handles.rio_dist        = cell( 1 );
handles.have_stations   = false;
handles.have_statistics = false;
handles.have_clcdist    = false;
handles.ignore_list     = [];
handles.stats_file      = '';

% Save the full arrays for later export...
handles.ignore_list  = ignore_list;

handles.rmse   = 0.; % RMSE for the trend line
handles.R2     = 0.; % R2 for the trend line
handles.BetaR2 = 0.; % R2 between the constucted beta parameter and the concentrations

handles.a_name    = { 'a_i' };
handles.beta_name = { '\beta' };

% Init the table
data = reset_ai_weights( handles.dpl.default_weights.(handles.pol_xx), handles.n_dim );
colname = cell(1, 4*handles.n_dim );
for i=1:handles.n_dim
    colname{4*i-3} = handles.a_name{i};
    colname{4*i-2} = 'use ?';
    colname{4*i-1} = 'lower';
    colname{4*i}   = 'upper';
end
set( handles.uitable2, 'Data', data );
set( handles.uitable2, 'ColumnName', colname );
set( handles.uitable2, 'ColumnEditable', true );

% get the datacursor mode and set the update function
handles.dcm_obj = datacursormode(hObject);
set(handles.dcm_obj,'UpdateFcn',{@dcmUpdateFcn,hObject});

guidata(hObject, handles);

% UIWAIT makes OptimizeGUI wait for user response (see UIRESUME)
% uiwait(handles.figure1);

return

function txt = dcmUpdateFcn( ~, evt, hFigure )
% get the axes where the user has clicked
hAxesParent  = get(get(evt,'Target'),'Parent');

% get the handles structure for the figure/GUI
handles      = guidata(hFigure);
 
% which axes are we in?
if hAxesParent==handles.axes1 % the trend plot
    pos = get(evt,'Position');
    txt = {['Beta: ',num2str(pos(1))],...
        ['C: ',num2str(pos(2))]};
    
    [ m, i ] = min( abs( handles.all_xx - pos(2) ) );
    
    if ( m > 1e-8 ) 
        error( 'unable to find station in data cursor mode' );
    end
    % find index of the station clicked
    stationName = handles.all_st_id{i,:};
    
    % do a figure with the class distribution
    dd = handles.all_rio_dist{1};
    bar(handles.axes3, 1:size(dd,2), dd(i,:) );
    set(handles.axes3,'XTick', 1:size(dd,2) );
    xlim(handles.axes3, [ 0.5 size(dd,2)+0.5 ] );
    xlabel(handles.axes3, stationName ); 

elseif hAxesParent==handles.axes2 % the rmse plot    
    txt = 'bias: ';
else
    % do nothing
end



% --- Outputs from this function are returned to the command line.
function varargout = OptimizeGUI_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

% --- Executes when entered data in editable cell(s) in uitable2.
function uitable2_CellEditCallback(hObject, eventdata, handles)
% hObject    handle to uitable2 (see GCBO)
% eventdata  structure with the following fields (see UITABLE)
%	Indices: row and column indices of the cell(s) edited
%	PreviousData: previous data for the cell(s) edited
%	EditData: string(s) entered by the user
%	NewData: EditData or its converted form set on the Data property. Empty if Data was not changed
%	Error: error string when failed to convert EditData to appropriate value for Data
% handles    structure with handles and user data (see GUIDATA)

if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end

a = get( hObject, 'Data' );
cla(handles.axes1,'reset');
cla(handles.axes2,'reset');

% get the requested plane type
switch( handles.fit_order )
    case 1
        plane = @(p,x) p(1)*x(:,1) + p(2)*x(:,2)+ p(3);
        ndf = 3;
    case 2
        plane = @(p,x) p(1)*x(:,1).^2 + p(2)*x(:,2).^2 + p(3)*x(:,1).*x(:,2) + p(4)*x(:,1) + p(5)*x(:,2)+ p(6);
        ndf = 6;
    otherwise
        errordlf( 'Unknown plane order !' );
        return
end

beta = zeros( handles.n_st, handles.n_dim );
for i=1:handles.n_dim
    beta(:,i) = calc_beta( a(:,4*i-3)', handles.rio_dist{i} );
end

% setup a xygrid (or xgrid) for plotting the trendline/surface
if ( handles.n_dim == 1 )
    tmp_beta  = linspace(0, max(beta), 50 )';
    p = polyfit( beta, handles.xx, handles.fit_order );
    st_type  = handles.st_info(:,4);
else
    [ tmp_beta1 tmp_beta2 ] = meshgrid( linspace( 0, max(beta(:,1)), 20 )', linspace( 0, max(beta(:,2)), 20 )' );
    tmp_beta = [ reshape( tmp_beta1, numel(tmp_beta1), 1 ) reshape(tmp_beta2,numel(tmp_beta2), 1 ) ];
    fit_opts = statset('nlinfit');
    fit_opts = statset(fit_opts, 'Robust', 'on' );
    p = nlinfit( beta, handles.xx, plane, zeros(1,ndf), fit_opts );
    st_type  = handles.st_info(:,6);
end

% Here we go...
axes(handles.axes1);


max_type = numel(handles.dpl.station_types);
type_lb = handles.dpl.station_types;
col_x = handles.dpl.station_type_marker; 
col_f = handles.dpl.station_type_color; 

shift_lb = max(beta)/50;

leg_disp = {};
leg_handle = [];

for i=1:max_type
    type_i = find(st_type == i);
    if ~isempty(type_i)
        if i>1, hold on; end
        if handles.n_dim == 1
          leg_handle(end+1) = plot(beta(type_i), handles.xx(type_i), char(col_x(i)), 'MarkerFaceColor', char(col_f(i)));
          leg_disp(end+1) = type_lb(i);
        else
            error( 'not supported...' );
          %scatter3( beta(type_i,1), beta(type_i,2), handles.xx(type_i), char(col_x(i)), 'MarkerFaceColor', char(col_f(i)));
        end
    end
end
legend( leg_handle, leg_disp, 'FontSize', 7, 'Location','Best' );

% Plot the labels, only for trendline... for now ;)
switch ( handles.labels )
    case 'ID'
        if ( handles.n_dim == 1 )
            text(beta+shift_lb, handles.xx, num2str(handles.st_info(:,1)), 'Fontsize', 8 , 'Color', 'k');
        else
            text(beta(:,1)+shift_lb(1), beta(:,2)+shift_lb(2), handles.xx, num2str(handles.st_info(:,1)), 'Fontsize', 8 , 'Color', 'k');
        end
    case 'Name'
        if ( handles.n_dim == 1 )
            text(beta+shift_lb, handles.xx, handles.st_id, 'Fontsize', 8 , 'Color', 'k');
        else 
           text(beta(:,1)+shift_lb(1), beta(:,2)+shift_lb(2), handles.xx, handles.st_id, 'Fontsize', 8 , 'Color', 'k'); 
        end
    otherwise
end


% And the plane...
if handles.n_dim == 1
    plot( tmp_beta, polyval( p, tmp_beta), 'r--' );
    xlabel( handles.beta_name{1} );
else
    plane_val = reshape( plane( p, tmp_beta ), size( tmp_beta1 ) );
    mesh( tmp_beta1, tmp_beta2, plane_val,'EdgeColor',[0 0 0],'FaceAlpha',0);
    xlabel( handles.beta_name{1}, 'Margin', 0.1 );
    ylabel( handles.beta_name{2} );
end
% set callback for mouse input
%set(handles.axes1, 'ButtonDownFcn', {@clickHandler,get(gca,'Position')});
hold off;

% Overall diagnostics
if handles.n_dim == 1
    handles.rmse = sqrt( sum( ( handles.xx - polyval( p, beta )).^2 ) / length( handles.xx ) );
    rho  = corrcoef( handles.xx, polyval( p, beta ) );
    handles.R2 = rho(1,2).^2;    
    rho = corrcoef( handles.xx, beta );
    handles.BetaR2 = rho(1,2).^2;
else 
    handles.rmse = sqrt( sum( ( handles.xx - plane( p, beta) ).^2 ) / length( handles.xx ) );
    rho  = corrcoef( handles.xx, plane( p, beta ) );
    handles.R2 = rho(1,2).^2;
    handles.BetaR2 = NaN;
end

% Now we plot for each station the deviation from the trendline...
axes(handles.axes2);
if handles.n_dim == 1
    st_res = handles.xx - polyval( p, beta ); 
else
    st_res = handles.xx - plane( p, beta );    
end
bar( st_res );
ylabel('\mug/m^3');
set(gca, 'XLim', [0.5 length(st_res)+0.5 ] );
set(gca, 'XTick', 1:length(st_res) );
set(gca, 'XTickLabel', char(handles.st_id), 'FontSize', 7 );
rotateticklabel( gca );
colormap summer; % ermm... don't ask :D

% Set overall diagnostics...
set( handles.edit1, 'String', sprintf( '%.4f', handles.rmse ) );
set( handles.edit2, 'String', sprintf( '%.4f', handles.R2 ) );
set( handles.edit7, 'String', sprintf( '%.4f', handles.BetaR2 ) );



guidata(hObject, handles);


% --- Executes on button press in pushbutton1.
function pushbutton1_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end


% get the a
data = get( handles.uitable2, 'data' );
if ( handles.n_dim == 1 )
    a     = data(:,1)';
    use_a = data(:,2)';
    cf.lb = data(:,3)';
    cf.ub = data(:,4)';
else
    %a     = [data(:,1) data(:,5)]';
    %use_a = [data(:,2) data(:,6)]';
    error( 'Optimisataion with double beta needs to be reimplemented....' );    
end

% minimisation config
cf.constrain       = get( handles.checkbox1, 'Value' );
cf.constrainFactor = str2double( get( handles.edit3, 'String' ) );

switch( get( handles.popupmenu6, 'Value' ) )
  case 1
    cf.norm = 'TRENDMSE';
  case 2
    cf.norm = 'TRENDCORR';
  case 3
    cf.norm = 'CORR';
end
cf.order    = handles.fit_order;

% This runs the optimisation
new_a = optimize_beta( a, use_a, handles.rio_dist, handles.xx, cf );

% Make a new data array with weights & use
data_new  = data; 
for i=1:handles.n_dim,
    data_new(:,4*i-3) = new_a(i,:)';
end
set( handles.uitable2, 'data', data_new );

% And replot
uitable2_CellEditCallback(handles.uitable2, eventdata, handles);

% And ask whether to keep the data ?
answer = questdlg('Keep the optimized weights ?', ...
    'Optimisation finished', 'No');
switch( answer )
    case 'No'
        % restore old values
        set( handles.uitable2, 'data', data );
        uitable2_CellEditCallback(handles.uitable2, eventdata, handles);
    otherwise
end

guidata(hObject, handles);


function edit1_Callback(hObject, eventdata, handles)
% hObject    handle to edit1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit1 as text
%        str2double(get(hObject,'String')) returns contents of edit1 as a double


% --- Executes during object creation, after setting all properties.
function edit1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit2_Callback(hObject, eventdata, handles)
% hObject    handle to edit2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit2 as text
%        str2double(get(hObject,'String')) returns contents of edit2 as a double


% --- Executes during object creation, after setting all properties.
function edit2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


function edit3_Callback(hObject, eventdata, handles)
% hObject    handle to edit3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit3 as text
%        str2double(get(hObject,'String')) returns contents of edit3 as a double


% --- Executes during object creation, after setting all properties.
function edit3_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in checkbox1.
function checkbox1_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox1


% --------------------------------------------------------------------
function uipushtool1_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to uipushtool1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end

data  = get( handles.uitable2, 'data' );
menu = get(handles.popupmenu4, 'String' );

% Get the RMSE & R2 currently on display
rmse = get( handles.edit1, 'String' );
R2   = get( handles.edit2, 'String' );
ttyp = menu{get(handles.popupmenu4, 'Value')};

[filename, pathname] = uiputfile('*.txt', 'Save optimized weigths...');
if ~( isequal(filename,0) || isequal(pathname,0) )
    fid = fopen( fullfile(pathname, filename), 'w' );
    fprintf( fid, '# Optimized RIO class weights a_i\n');
    fprintf( fid, '# GIS type : %s\n', handles.gis_type );
    fprintf( fid, '# Pollutant : %s\n', handles.pol_xx );
    fprintf( fid, '# Long term station data : %s\n', handles.stats_file );
    fprintf( fid, '# Stations not used :' );
    for k=1:length( handles.ignore_list )
        fprintf( fid, ' %s (%d)' , char(handles.all_st_id( handles.ignore_list(k) )), handles.ignore_list(k));
    end
    fprintf( fid, '\n' );
    fprintf( fid, '# Trend type : %s\n', ttyp );
    fprintf( fid, '# RMSE = %s\n', rmse );
    fprintf( fid, '# R2   = %s\n', R2 );
    fprintf( fid, '# I' );
    for j=1:handles.n_dim, fprintf( fid, ',%s', handles.a_name{j} ); end;
    fprintf( fid, '\n' );
    for i=1:length( handles.rio_class_names )
        fprintf( fid, '%d', i );
        for j=1:handles.n_dim, fprintf( fid, ',%.4f', data( i, 4*j-3 ) ); end;
        fprintf( fid, '\n' );
    end
    fclose(fid);
end



% --------------------------------------------------------------------
function uitoggletool6_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to uitoggletool6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

str = handles.rio_class_names;
for i=1:length(str), 
   str{i} = strcat( sprintf( '[a%2d] ', i ), str{i} );  
end
msgbox( str, 'RIO Classes Legend' );
guidata( hObject, handles );




% --- Executes when selected object is changed in uipanel4.
function uipanel4_SelectionChangeFcn(hObject, eventdata, handles)
% hObject    handle to the selected object in uipanel4 
% eventdata  structure with the following fields (see UIBUTTONGROUP)
%	EventName: string 'SelectionChanged' (read only)
%	OldValue: handle of the previously selected object or empty if none was selected
%	NewValue: handle of the currently selected object
% handles    structure with handles and user data (see GUIDATA)
switch get(eventdata.NewValue,'Tag') % Get Tag of selected object.
    case 'radiobutton1'
        handles.labels = 'ID';
    case 'radiobutton2'
        handles.labels = 'Name';
    case 'radiobutton3'
         handles.labels = 'None';
    otherwise
        handles.labels = 'None';
end
guidata( hObject, handles );
uitable2_CellEditCallback(handles.uitable2, eventdata, handles);


% --------------------------------------------------------------------
function uipushtool3_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to uipushtool3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end

data  = get( handles.uitable2, 'data' );

% This retreives an existing weights file and loads it into the table...
[filename, pathname] = uigetfile('*.txt', 'Load optimized weights...');
if ~( isequal(filename,0) || isequal(pathname,0) )
    tmp = importdata( fullfile( pathname, filename ) );
    if size( tmp.data ) ~= [ size(data,1) 3 ]
       errordlg( 'Unable to load this file, check the format !', 'Unable to load file...', 'modal' );
    else
        for k=1:handles.n_dim,
            data(:,4*k-3) = tmp.data(:,k+1);
        end

        set( handles.uitable2, 'data', data );
        guidata( hObject, handles );
        uitable2_CellEditCallback(handles.uitable2, eventdata, handles);
    end
end


% --------------------------------------------------------------------
function uipushtool4_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to uipushtool4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end


[filename, pathname] = uiputfile('*.emf', 'Save optimized trend figure...');
if ~( isequal(filename,0) || isequal(pathname,0) )
    % This one saves the optimisation plot, cannot export axes directly so
    % have to copy the object to a new figure and save that....
    % create a new figure for saving and printing
    fig = figure('visible','off');
    % copy axes into the new figure
    axes( handles.axes1 );
    newax = copyobj( gca , fig );
    set(newax, 'units', 'normalized', 'position', [0.13 0.11 0.775 0.815]);
    % print and/or save the figure
    saveas( fig, fullfile( pathname, filename ), 'emf' ); % save it
    close(fig) % clean up by closing it
end


% --- Executes on button press in checkbox2.
function checkbox2_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox2


% --- Executes on selection change in popupmenu4.
function popupmenu4_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmenu4 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu4
if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end


menu = get(hObject, 'String' );
sel = menu{get(hObject,'Value')};

fprintf( 'Changed trend type to %s\n', sel );
switch( sel )
    case 'Linear'
        handles.fit_order = 1;
    case 'Quadratic'
        handles.fit_order = 2;
    otherwise
        fprintf( '*** Unknown order trend type %s\n', sel );
        handles.fit_order(1) = -1;
end
guidata( hObject, handles );
uitable2_CellEditCallback(handles.uitable2, eventdata, handles);




% --- Executes during object creation, after setting all properties.
function popupmenu4_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --------------------------------------------------------------------
function uipushtool5_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to uipushtool5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)



% --------------------------------------------------------------------
function ImportMenu_Callback(hObject, eventdata, handles)
% hObject    handle to ImportMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function LoadStations_Callback(hObject, eventdata, handles)
% hObject    handle to LoadStations (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[fname, path ] = uigetfile( {'*.txt;*.asc;*.dat', 'Station files (*.txt, *.asc, *.dat)'; ...
    '*.*', 'All Files (*.*)'}, 'Pick a station file' );
if isequal(fname,0) || isequal(path,0)
    return
end
station_info_file = fullfile( path, fname );

db = importdata( station_info_file );
handles.st_id = db.textdata(2:end,2);

% depending on the number of columns : we have the altitude in the input files or not... 
if size(db.data,2) == 5
    handles.st_info  = [ str2double( db.textdata(2:end,1) ) db.data(:,1:2) db.data(:,4) db.data(:,5) ];
elseif size(db.data,2) == 4
    warning( '+++ Reading old station format without the altitude field...' );
    handles.st_info  = [ str2double( db.textdata(2:end,1) ) db.data(:,1:2) db.data(:,3) db.data(:,4) ];
end

%-- Convert to cell array
handles.all_st_id     = handles.st_id;
handles.all_st_info   = handles.st_info;
handles.have_stations = true;
clear db;

set(handles.edit8,'String', fullfile( path, fname ) );
guidata( hObject, handles );

if ( handles.have_stations  && handles.have_clcdist && handles.have_statistics )
    uitable2_CellEditCallback(handles.uitable2, eventdata, handles);
end



% --------------------------------------------------------------------
function LoadFootprint_Callback(hObject, eventdata, handles)
% hObject    handle to LoadFootprint (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[fname, path ] = uigetfile( {'*.txt;*.asc;*.dat', 'CLC footprints (*.txt, *.asc, *.dat)'; ...
    '*.*', 'All Files (*.*)'}, 'Pick a station footprint file' );
if isequal(fname,0) || isequal(path,0)
    return
end
imp_file_gis = fullfile( path, fname );

fprintf( 'Loading %s\n', imp_file_gis );
tmp = importdata(imp_file_gis, ';', 1 );

if size( tmp.data, 1 ) ~= size( handles.st_id, 1 )
    errordlg( sprintf( 'Number of stations do not match in %s\n', imp_file_gis ), ...
        'RIO Beta optimisation error', 'modal');
    return;
end
handles.rio_dist{1}     = reshape_gis_data( tmp.data(:,4:end), handles.rio_class_agg);
handles.all_rio_dist{1} = handles.rio_dist{1};

handles.have_clcdist = true;
set(handles.edit9,'String', fullfile( path, fname ) );

guidata( hObject, handles );

if ( handles.have_stations  && handles.have_clcdist && handles.have_statistics )
    uitable2_CellEditCallback(handles.uitable2, eventdata, handles);
end



% --------------------------------------------------------------------
function LoadStats_Callback(hObject, eventdata, handles)
% hObject    handle to LoadStats (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%-- Load TS expectation value...

[fname, path ] = uigetfile( {'*.mat', 'Statistics files (*.mat)'; ...
    '*.*', 'All Files (*.*)'}, 'Pick long term averages file...' );
if isequal(fname,0) || isequal(path,0)
    return
end
handles.stats_file   = fullfile( path, fname );
x              = load( handles.stats_file );        
handles.xx     = x.xx_avg(:,2);
handles.all_xx = handles.xx;
handles.n_st = size( handles.xx, 1 );

handles.have_statistics = true;
 
set(handles.edit10,'String', fullfile( path, fname ) );

guidata( hObject, handles );

if ( handles.have_stations  && handles.have_clcdist && handles.have_statistics )
    uitable2_CellEditCallback(handles.uitable2, eventdata, handles);
end


% --------------------------------------------------------------------
function ConfigStations_Callback(hObject, eventdata, handles)
% hObject    handle to ConfigStations (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end

handles.ignore_list = configure_stations( handles.all_st_id, handles.all_st_info, handles.ignore_list );

handles.xx      = handles.all_xx;
handles.st_id   = handles.all_st_id;
handles.st_info = handles.all_st_info;

handles.xx( handles.ignore_list )            = [];
handles.st_id( handles.ignore_list )         = [];
handles.st_info( handles.ignore_list, : )    = [];
for i=1:handles.n_dim, 
    handles.rio_dist{i} = handles.all_rio_dist{i};
    handles.rio_dist{i}( handles.ignore_list, : ) = []; 
end;
handles.n_st = size( handles.xx, 1 );

guidata( hObject, handles );
uitable2_CellEditCallback(handles.uitable2, eventdata, handles);


% --------------------------------------------------------------------
function ExportMenu_Callback(hObject, eventdata, handles)
% hObject    handle to ExportMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function ExportGrid_Callback(hObject, eventdata, handles)
% hObject    handle to ExportGrid (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Export a grid file with the current ai parameters...
% Load the class distribution files
% This retreives an existing weights file and loads it into the table...
if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end

% Book the grid distribution
grid_dist = cell(1,handles.n_dim);
beta      = cell(1,handles.n_dim);

% Get the a parameters...
data = get( handles.uitable2, 'data' );
if ( handles.n_dim == 1 )
    a = data(:,1)';
else
    a = [data(:,1) data(:,5)]';
end

for k=1:handles.n_dim
    str = sprintf( 'Load grid class distribution file...' );
    [filename, pathname] = uigetfile('*.txt', str );
    if ~( isequal(filename,0) || isequal(pathname,0) )        
        tmp = importdata( fullfile( pathname, filename ), ';', 1 );
        grid_dist{k} = reshape_gis_data( tmp.data(:,4:end), handles.rio_class_agg );
        grid_id      = tmp.data(:,1);
    end
    
    % Calculate beta for this set of classes...
    beta{k} = calc_beta( a(k,:), grid_dist{k} );
end

% Now export the beta maps
[filename, pathname] = uiputfile('*.txt', 'Save new grid file...' );
if ~( isequal(filename,0) || isequal(pathname,0) )
    % Export a file with new beta parameters
    fid = fopen( fullfile( pathname, filename ), 'w' );
    fprintf( fid, 'ID' );
    for k=1:handles.n_dim
        fprintf( fid, ';%s', handles.beta_name{k} );
    end
    fprintf( fid, '\n' );
    for i=1:length( grid_id )
        fprintf( fid, '%d', grid_id(i) );
        for k=1:handles.n_dim
            fprintf( fid, ';%.4f', beta{k}(i) );
        end
        fprintf( fid, '\n' );
    end
end


% --------------------------------------------------------------------
function ExportStations_Callback(hObject, eventdata, handles)
% hObject    handle to ExportStations (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end

[filename, pathname] = uiputfile('*.txt', 'Save stations beta file');

% get the a
data = get( handles.uitable2, 'data' );
if ( handles.n_dim == 1 )
    a = data(:,1)';
else
    a = [data(:,1) data(:,5)]';
end

beta = zeros( length( handles.all_st_id ), handles.n_dim );
for i=1:handles.n_dim
    beta(:,i) = calc_beta( a(i,:), handles.all_rio_dist{i} );
end

if ~( isequal(filename,0) || isequal(pathname,0) )
    % Export a file with new beta parameters
    fid = fopen( fullfile( pathname, filename ), 'w' );
    fprintf( fid, 'ID\tSTATCODE\tXLAMB\tYLAMB\tTYPE' );
    if( handles.n_dim == 2 )
        fprintf( fid, '\tBeta500m\tBeta5km\n' );
    else
        fprintf( fid, '\tBETA\n' );
    end
    for i=1:length(handles.all_st_id)
        fprintf( fid, '%d\t%s\t%d\t%d', handles.all_st_info(i,1), char(handles.all_st_id(i)), ...
            handles.all_st_info(i,2), handles.all_st_info(i,3) );
        if( handles.n_dim == 2 )
            fprintf( fid, '\t%d\t%f\t%f\n', handles.all_st_info(i,6), beta(i,1), beta(i,2) );
        else
            fprintf( fid, '\t%d\t%f\n', handles.all_st_info(i,5), beta(i,1) );
        end
    end
    
    fclose( fid );
end


% --- Executes on button press in resetButton.
function resetButton_Callback(hObject, eventdata, handles)
% hObject    handle to resetButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Init the table
if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end

data = reset_ai_weights( handles.dpl.default_weights.(handles.pol_xx), handles.n_dim );
colname = cell(1, 4*handles.n_dim );
for i=1:handles.n_dim
    colname{4*i-3} = handles.a_name{i};
    colname{4*i-2} = 'use ?';
    colname{4*i-1} = 'lower';
    colname{4*i}   = 'upper';
end
set( handles.uitable2, 'Data', data );

uitable2_CellEditCallback(handles.uitable2, eventdata, handles);

guidata( hObject, handles );



function edit7_Callback(hObject, eventdata, handles)
% hObject    handle to edit7 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit7 as text
%        str2double(get(hObject,'String')) returns contents of edit7 as a double


% --- Executes during object creation, after setting all properties.
function edit7_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit7 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit4_Callback(hObject, eventdata, handles)
% hObject    handle to edit4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit4 as text
%        str2double(get(hObject,'String')) returns contents of edit4 as a double


% --- Executes during object creation, after setting all properties.
function edit4_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit5_Callback(hObject, eventdata, handles)
% hObject    handle to edit5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit5 as text
%        str2double(get(hObject,'String')) returns contents of edit5 as a double


% --- Executes during object creation, after setting all properties.
function edit5_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in popupmenu6.
function popupmenu6_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns popupmenu6 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu6


% --- Executes during object creation, after setting all properties.
function popupmenu6_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton5.
function pushbutton5_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% set the constraints and load them into the table
% afterwards a user can still edit the boundaries...
if ~handles.have_stations || ~handles.have_statistics || ~ handles.have_clcdist
  errordlg( 'Load station, statistics and class distribution first', 'Setup incomplete', 'modal' );
  return;
end

% Make a new data array with weights & use
fac = str2double( get( handles.edit3, 'String' ) );
data = get(handles.uitable2,'data');
for i=1:handles.n_dim,
    data(:,4*i-1) = data(:,4*i-3)-0.01*fac*data(:,4*i-3);
    data(:,4*i)   = data(:,4*i-3)+0.01*fac*data(:,4*i-3);
    
    % restrict to positive weights
    data( data(:,4*i-1) < 0, 4*i-1) = 0;
end
set( handles.uitable2, 'data', data );
uitable2_CellEditCallback(handles.uitable2, eventdata, handles);



function edit8_Callback(hObject, eventdata, handles)
% hObject    handle to edit8 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit8 as text
%        str2double(get(hObject,'String')) returns contents of edit8 as a double


% --- Executes during object creation, after setting all properties.
function edit8_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit8 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit9_Callback(hObject, eventdata, handles)
% hObject    handle to edit9 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit9 as text
%        str2double(get(hObject,'String')) returns contents of edit9 as a double


% --- Executes during object creation, after setting all properties.
function edit9_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit9 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit10_Callback(hObject, eventdata, handles)
% hObject    handle to edit10 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit10 as text
%        str2double(get(hObject,'String')) returns contents of edit10 as a double


% --- Executes during object creation, after setting all properties.
function edit10_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit10 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
