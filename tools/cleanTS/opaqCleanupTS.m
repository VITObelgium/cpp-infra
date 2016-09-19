function varargout = opaqCleanupTS(varargin)
% opaqCleanupTS MATLAB code for opaqCleanupTS.fig
%      opaqCleanupTS, by itself, creates a new opaqCleanupTS or raises the existing
%      singleton*.
%
%      H = opaqCleanupTS returns the handle to a new opaqCleanupTS or the handle to
%      the existing singleton*.
%
%      opaqCleanupTS('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in opaqCleanupTS.M with the given input arguments.
%
%      opaqCleanupTS('Property','Value',...) creates a new opaqCleanupTS or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before opaqCleanupTS_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to opaqCleanupTS_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help opaqCleanupTS

% Last Modified by GUIDE v2.5 27-Mar-2014 14:31:16

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @opaqCleanupTS_OpeningFcn, ...
                   'gui_OutputFcn',  @opaqCleanupTS_OutputFcn, ...
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


% --- Executes just before opaqCleanupTS is made visible.
function opaqCleanupTS_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to opaqCleanupTS (see VARARGIN)

handles.have_data = false;
handles.fname = '';

handles.xx_date    = []; % array with dates
handles.xx_vals    = []; % original values
handles.xx_corr    = []; % corrected values cell array 
handles.corr_table = []; % correction table

% reset axis
handles.have_axis  = false;

% Choose default command line output for opaqCleanupTS
handles.output = hObject;

% Set adaptive date ticks, adjusted from 
% http://www.mathworks.com/matlabcentral/fileexchange/15342-zoom-adaptive-date-ticks
% zoomAdaptiveDateTicks( 'on' );
% also do this for the pan function...
set(zoom(gcf),'ActionPostCallback', @adaptiveDateTicks);
set(pan(gcf),'ActionPostCallback', @adaptiveDateTicks);

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes opaqCleanupTS wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = opaqCleanupTS_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --------------------------------------------------------------------
function FileMenu_Callback(hObject, eventdata, handles)
% hObject    handle to FileMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function OpenFile_Callback(hObject, eventdata, handles)
% hObject    handle to OpenFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[filename, pathname] = uigetfile( ...
{'*.txt;*.asc;*.asc;*.dat','Text files (*.txt,*.asc,*.dat)';   
   '*.*',  'All Files (*.*)'}, ...
   'Pick an air quality data file...', handles.fname );
if isequal(filename,0)
    disp('Cancelled by user...');
    return;
end

try
    [ handles.xx_date, handles.xx_vals ] = ...
        read_aqdata( fullfile(pathname, filename ), ...
        'format', 'libovito', 'Reshape1D', true, 'Timestamp', 'after', 'Trim', true );
catch MEx
    errordlg( MEx.message, 'Error opening file...', 'modal' );
    return;
end
% reset axis
handles.have_axis = false;

% set initial correcte is xx_vals
handles.xx_corr = handles.xx_vals;
handles.corr_table{1} = 'original';

% open
handles.fname = fullfile( pathname, filename );
set( handles.fileNameEdit, 'String', filename );

% try to parse station, index and pollutant from filename
str   = filename;
s_pol = get( handles.polMenu, 'String' );
[ startIdx, endIdx ] = regexpi( filename, '(pm10|pm25|no2|o3|co|so2|bzn|co2|ec|bc)' );
if ~isempty( startIdx ) && ~isempty( endIdx )
    idx = find( strcmpi( s_pol, str(startIdx(1):endIdx(1))) );
    if ~isempty( idx )
        fprintf( 'Guessing pollutant in this file is : %s\n', str(startIdx(1):endIdx(1)));
        set( handles.polMenu, 'Value', idx );
    end
    
    for k=1:length(startIdx)
        str(startIdx(k):endIdx(k)) = '_';
    end
end

% kill '_,-,etc' to strip for the station name
str = strrep( str, '_', '' );
str = strrep( str, '-', '' );
[~,st_name,ext] = fileparts(str);
fprintf( 'Guessing station name : %s\n', st_name );
set( handles.stationEdit, 'String', st_name );

% guessing station idx from name
[ startIdx, endIdx ] = regexp( st_name, '([0-9]*)');
if ~isempty(startIdx) && ~isempty(endIdx)
    set( handles.stationIdxEdit, 'String', num2str( str2double( st_name(startIdx(end):endIdx(end))) )  );
else
     set( handles.stationIdxEdit, 'String', 'NaN' );
end

% make plot
handles.have_data = true;
makePlot(hObject, eventdata, handles);
guidata(hObject,handles);


% --------------------------------------------------------------------
function ExportOVL_Callback(hObject, eventdata, handles)
% hObject    handle to ExportOVL (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)



function fileNameEdit_Callback(hObject, eventdata, handles)
% hObject    handle to fileNameEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of fileNameEdit as text
%        str2double(get(hObject,'String')) returns contents of fileNameEdit as a double


% --- Executes during object creation, after setting all properties.
function fileNameEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to fileNameEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in polMenu.
function polMenu_Callback(hObject, eventdata, handles)
% hObject    handle to polMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns polMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from polMenu


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


% --- Executes on button press in plotButton.
function makePlot(hObject, eventdata, handles)
% hObject    handle to plotButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

axes( handles.plot_axes );

axis manual;

s = get( handles.polMenu, 'String');
pol = s{ get( handles.polMenu, 'Value' )};
station = get( handles.stationEdit, 'String' );

if ~handles.have_data
    errordlg( 'Please load a data file first...', 'Error plotting data...', 'modal' );
    return;
end

% save axis ranges
xrange = get( handles.plot_axes, 'XLim' );
yrange = get( handles.plot_axes, 'YLim' );

% here we always plot the last correction
plot(handles.plot_axes, handles.xx_date, handles.xx_vals, 'Color', [ .7 .7 .7 ] );
hold on;
plot(handles.plot_axes, handles.xx_date, handles.xx_corr(:,end), 'Color', [ .9 .1 .1 ] );
hold off;

% set old axis ranges
if ~handles.have_axis
    xrange = get( handles.plot_axes, 'XLim' );
    yrange = get( handles.plot_axes, 'YLim' );
    handles.have_axis = true;
else
    axis(handles.plot_axes, [ xrange yrange ]);
end

datetick(handles.plot_axes, 'keeplimits', 'keepticks' );
grid on;
xlabel( handles.plot_axes, sprintf( 'DateTime from %s to %s', datestr(xrange(1)), datestr(xrange(2)) ) );
ylabel( handles.plot_axes, sprintf( '%s - %s [µg/m^3]', upper(station), upper(pol) ) );


legend( handles.plot_axes, 'Original', 'Corrected' );
guidata(hObject, handles );


function stationEdit_Callback(hObject, eventdata, handles)
% hObject    handle to stationEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of stationEdit as text
%        str2double(get(hObject,'String')) returns contents of stationEdit as a double


% --- Executes during object creation, after setting all properties.
function stationEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to stationEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in delvalActive.
function delvalActive_Callback(hObject, eventdata, handles)
% hObject    handle to delvalActive (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of delvalActive



function hardlimLowerEdit_Callback(hObject, eventdata, handles)
% hObject    handle to hardlimLowerEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of hardlimLowerEdit as text
%        str2double(get(hObject,'String')) returns contents of hardlimLowerEdit as a double


% --- Executes during object creation, after setting all properties.
function hardlimLowerEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to hardlimLowerEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function hardlimUpperEdit_Callback(hObject, eventdata, handles)
% hObject    handle to hardlimUpperEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of hardlimUpperEdit as text
%        str2double(get(hObject,'String')) returns contents of hardlimUpperEdit as a double


% --- Executes during object creation, after setting all properties.
function hardlimUpperEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to hardlimUpperEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in applyHardLimButton.
function applyHardLimButton_Callback(hObject, eventdata, handles)
% hObject    handle to applyHardLimButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

lo = str2double( get(handles.hardlimLowerEdit, 'String' ) );
hi = str2double( get(handles.hardlimUpperEdit, 'String' ) );

[ dummy, new_corr ] = tscorr_hardlim( handles.xx_date, handles.xx_corr(:,end), lo, hi );

% add a new correction
handles.xx_corr(:,end+1) = new_corr;
handles.corr_table{end+1} = sprintf( 'hardlim, lo=%.1f, hi=%.1f', lo, hi );
print_corrtable( handles.corr_table );
makePlot(hObject, eventdata, handles);
guidata(hObject,handles);


function print_corrtable( T )
fprintf( 'APPLIED CORRECTIONS TABLE:\n' );
for i=1:length(T)
    fprintf( '[%d] %s\n', i-1, T{i} );
end


% --- Executes on button press in undoButton.
function undoButton_Callback(hObject, eventdata, handles)
% hObject    handle to undoButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% pop the last correction from the list
if size( handles.xx_corr, 2 ) > 1
    handles.xx_corr(:,end) = [];
    handles.corr_table(end) = [];
    print_corrtable( handles.corr_table );
    makePlot(hObject, eventdata, handles);
else
    warndlg( 'No more corrections to undo...', 'Error undo correction', 'modal' );
end
guidata(hObject,handles);


% --- Executes on button press in resetButton.
function resetButton_Callback(hObject, eventdata, handles)
% hObject    handle to resetButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.have_axis = false; % reset axis
handles.xx_corr = handles.xx_vals;
handles.corr_table = [];
handles.corr_table{1} = 'original';
print_corrtable( handles.corr_table );
makePlot(hObject, eventdata, handles);
guidata(hObject,handles);


function adaptiveDateTicks(figureHandle,eventObjectHandle)
% Resetting x axis to automatic tick mark generation 
set(eventObjectHandle.Axes,'XTickMode','auto')
% using automaticallly generate date ticks
datetick(eventObjectHandle.Axes,'x','keeplimits')
% Adde by Bino : 
% also set the axis xlabel
xrange = get( eventObjectHandle.Axes, 'XLim' );
xlabel( eventObjectHandle.Axes, ...
    sprintf( 'DateTime from %s to %s', datestr(xrange(1)), datestr(xrange(2)) ) );


% --- Executes on button press in hfLogTransCheck.
function hfLogTransCheck_Callback(hObject, eventdata, handles)
% hObject    handle to hfLogTransCheck (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of hfLogTransCheck



function hfWindowWidthEdit_Callback(hObject, eventdata, handles)
% hObject    handle to hfWindowWidthEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of hfWindowWidthEdit as text
%        str2double(get(hObject,'String')) returns contents of hfWindowWidthEdit as a double


% --- Executes during object creation, after setting all properties.
function hfWindowWidthEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to hfWindowWidthEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function hfTEdit_Callback(hObject, eventdata, handles)
% hObject    handle to hfTEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of hfTEdit as text
%        str2double(get(hObject,'String')) returns contents of hfTEdit as a double


% --- Executes during object creation, after setting all properties.
function hfTEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to hfTEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in hfButton.
function hfButton_Callback(hObject, eventdata, handles)
% hObject    handle to hfButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

logtrans = get( handles.hfLogTransCheck, 'Value' );
DX = str2double( get(handles.hfWindowWidthEdit, 'String' ) );
T  = str2double( get(handles.hfTEdit, 'String' ) );

[ dummy, new_corr ] = tscorr_hampel( handles.xx_date, handles.xx_corr(:,end), logtrans, DX, T );

% add a new correction
handles.xx_corr(:,end+1) = new_corr;
handles.corr_table{end+1} = sprintf( 'hampel, logtrans=%d, DX=%.3f, T=%.3f', logtrans, DX, T );
print_corrtable( handles.corr_table );
makePlot(hObject, eventdata, handles);
guidata(hObject,handles);


% --- Executes on selection change in gadTransfMenu.
function gadTransfMenu_Callback(hObject, eventdata, handles)
% hObject    handle to gadTransfMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns gadTransfMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from gadTransfMenu


% --- Executes during object creation, after setting all properties.
function gadTransfMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to gadTransfMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in gadTransfCheckButton.
function gadTransfCheckButton_Callback(hObject, eventdata, handles)
% hObject    handle to gadTransfCheckButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


nbins = str2double( get( handles.gadBinsEdit, 'String' ) );
alph  = str2double( get( handles.gadAlphaEdit, 'String' ) );
 
% now limit the date range
d1 = handles.xx_date(1);
d2 = handles.xx_date(end);

if ~isempty( get( handles.gadFromEdit, 'String' ) )
    d1 = datenum( get( handles.gadFromEdit, 'String' ) );
end
if ~isempty( get( handles.gadToEdit, 'String' ) )
    d2 = datenum( get( handles.gadToEdit, 'String' ) );
end


opts = struct( ...
    'start_date', d1, ...
    'end_date', d2, ...
    'transform_idx', get( handles.gadTransfMenu,'Value'), ...
    'transform_p', str2double(get( handles.transfParEdit,'String')), ...
    'nbins', nbins, ...
    'alpha', alph, ...
    'make_plot', true, ...
    'apply_lower', false, ...
    'apply_upper', false );

% just run this without applying... just a check bascially
tscorr_gaussiananomaly( handles.xx_date, handles.xx_corr(:,end), opts );



function gadBinsEdit_Callback(hObject, eventdata, handles)
% hObject    handle to gadBinsEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of gadBinsEdit as text
%        str2double(get(hObject,'String')) returns contents of gadBinsEdit as a double


% --- Executes during object creation, after setting all properties.
function gadBinsEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to gadBinsEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


function gadAlphaEdit_Callback(hObject, eventdata, handles)
% hObject    handle to gadAlphaEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of gadAlphaEdit as text
%        str2double(get(hObject,'String')) returns contents of gadAlphaEdit as a double


% --- Executes during object creation, after setting all properties.
function gadAlphaEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to gadAlphaEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function gadFromEdit_Callback(hObject, eventdata, handles)
% hObject    handle to gadFromEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of gadFromEdit as text
%        str2double(get(hObject,'String')) returns contents of gadFromEdit as a double


% --- Executes during object creation, after setting all properties.
function gadFromEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to gadFromEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function gadToEdit_Callback(hObject, eventdata, handles)
% hObject    handle to gadToEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of gadToEdit as text
%        str2double(get(hObject,'String')) returns contents of gadToEdit as a double


% --- Executes during object creation, after setting all properties.
function gadToEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to gadToEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in gadRangeSetButton.
function gadRangeSetButton_Callback(hObject, eventdata, handles)
% hObject    handle to gadRangeSetButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


if isfield( handles, 'gadLine1' ) && ishandle( handles.gadLine1 )
    delete( handles.gadLine1 );
end
if isfield( handles, 'gadLine2' )  && ishandle( handles.gadLine2 )
    delete( handles.gadLine2 );
end

% get 2 points mouse input for training ranges
[ x, y ] = ginput(2);

set( handles.gadFromEdit, 'String', datestr( min(x) ) );
set( handles.gadToEdit, 'String', datestr( max(x) ) );

axes(handles.plot_axes);
yl = get(handles.plot_axes, 'YLim' ) ;

hold on;
handles.gadLine1 = line( [ min(x) min(x) ], yl, 'LineStyle', ':', 'Color', 'b' );
handles.gadLine2 = line( [ max(x) max(x) ], yl, 'LineStyle', ':', 'Color', 'b' );

guidata(hObject,handles);


% --- Executes on button press in gadApplyButton.
function gadApplyButton_Callback(hObject, eventdata, handles)
% hObject    handle to gadApplyButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

nbins = str2double( get( handles.gadBinsEdit, 'String' ) );
alph  = str2double( get( handles.gadAlphaEdit, 'String' ) );
 
% now limit the date range
d1 = handles.xx_date(1);
d2 = handles.xx_date(end);

if ~isempty( get( handles.gadFromEdit, 'String' ) )
    d1 = datenum( get( handles.gadFromEdit, 'String' ) );
end
if ~isempty( get( handles.gadToEdit, 'String' ) )
    d2 = datenum( get( handles.gadToEdit, 'String' ) );
end

% build options structure
opts = struct( ...
    'start_date', d1, ...
    'end_date', d2, ...
    'transform_idx', get( handles.gadTransfMenu,'Value'), ...
    'transform_p', str2double(get( handles.transfParEdit,'String')), ...
    'nbins', nbins, ...
    'alpha', alph, ...
    'make_plot', true, ...
    'apply_lower', get( handles.gadLowerCheckBox, 'Value' ), ...
    'apply_upper', get( handles.gadUpperCheckBox, 'Value' ) );


% just run this without applying... just a check bascially
[ dummy, new_corr ] = tscorr_gaussiananomaly( handles.xx_date, handles.xx_corr(:,end), opts );

if isfield( handles, 'gadLine1' ) && ishandle( handles.gadLine1 )
    delete( handles.gadLine1 );
end
if isfield( handles, 'gadLine2' )  && ishandle( handles.gadLine2 )
    delete( handles.gadLine2 );
end

% add a new correction
handles.xx_corr(:,end+1) = new_corr;
handles.corr_table{end+1} = sprintf( 'gaussian_anomaly, d1=%s, d2=%s, transform=%d, alpha=%f, lower=%d, upper=%d', ...
    datestr(d1), datestr(d2), opts.transform_idx, alph, ...
    get( handles.gadLowerCheckBox, 'Value' ), get( handles.gadUpperCheckBox, 'Value' ) );
print_corrtable( handles.corr_table );
makePlot(hObject, eventdata, handles);
guidata(hObject,handles);


% --- Executes on button press in gadLowerCheckBox.
function gadLowerCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to gadLowerCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of gadLowerCheckBox


% --- Executes on button press in gadUpperCheckBox.
function gadUpperCheckBox_Callback(hObject, eventdata, handles)
% hObject    handle to gadUpperCheckBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of gadUpperCheckBox


% --- Executes on button press in filleNanButton.
function filleNanButton_Callback(hObject, eventdata, handles)
% hObject    handle to filleNanButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


s = get( handles.nanMethodMenu, 'String' );
idx = get( handles.nanMethodMenu, 'Value' );

opts = struct( 'method', strtrim(s{idx}) );
[ dummy, new_corr ] = tscorr_fillnan( handles.xx_date, handles.xx_corr(:,end), opts );

% add a new correction
handles.xx_corr(:,end+1) = new_corr;
handles.corr_table{end+1} = sprintf( 'fill_nan, method=%s', strtrim(s{idx}) );    
print_corrtable( handles.corr_table );
makePlot(hObject, eventdata, handles);
guidata(hObject,handles);



function nanMethodMenu_Callback(hObject, eventdata, handles)
% hObject    handle to nanMethodMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of nanMethodMenu as text
%        str2double(get(hObject,'String')) returns contents of nanMethodMenu as a double


% --- Executes during object creation, after setting all properties.
function nanMethodMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to nanMethodMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in rmPeriodButton.
function rmPeriodButton_Callback(hObject, eventdata, handles)
% hObject    handle to rmPeriodButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% now limit the date range
d1 = handles.xx_date(1);
d2 = handles.xx_date(end);

if ~isempty( get( handles.rmDateFromEdit, 'String' ) )
    d1 = datenum( get( handles.rmDateFromEdit, 'String' ) );
end
if ~isempty( get( handles.rmDateToEdit, 'String' ) )
    d2 = datenum( get( handles.rmDateToEdit, 'String' ) );
end


opts = struct( 'start_date', d1, 'end_date', d2 );
[ dummy, new_corr ] = tscorr_removedates( handles.xx_date, handles.xx_corr(:,end), opts );

% remove the lines
if isfield( handles, 'rmDateLine1' ) && ishandle( handles.rmDateLine1 )
    delete( handles.rmDateLine1 );
end
if isfield( handles, 'rmDateLine2' )  && ishandle( handles.rmDateLine2 )
    delete( handles.rmDateLine2 );
end


% add a new correction
handles.xx_corr(:,end+1) = new_corr;
handles.corr_table{end+1} = sprintf( 'remove_dates, from=%s, to=%s', datestr(d1), datestr(d2) );    
print_corrtable( handles.corr_table );
makePlot(hObject, eventdata, handles);
guidata(hObject,handles);



function rmDateFromEdit_Callback(hObject, eventdata, handles)
% hObject    handle to rmDateFromEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of rmDateFromEdit as text
%        str2double(get(hObject,'String')) returns contents of rmDateFromEdit as a double


% --- Executes during object creation, after setting all properties.
function rmDateFromEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to rmDateFromEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function rmDateToEdit_Callback(hObject, eventdata, handles)
% hObject    handle to rmDateToEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of rmDateToEdit as text
%        str2double(get(hObject,'String')) returns contents of rmDateToEdit as a double


% --- Executes during object creation, after setting all properties.
function rmDateToEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to rmDateToEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in rmDatesSetButton.
function rmDatesSetButton_Callback(hObject, eventdata, handles)
% hObject    handle to rmDatesSetButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if isfield( handles, 'rmDateLine1' ) && ishandle( handles.rmDateLine1 )
    delete( handles.rmDateLine1 );
end
if isfield( handles, 'rmDateLine2' )  && ishandle( handles.rmDateLine2 )
    delete( handles.rmDateLine2 );
end

% get 2 points mouse input for training ranges
[ x, y ] = ginput(2);

set( handles.rmDateFromEdit, 'String', datestr( min(x) ) );
set( handles.rmDateToEdit, 'String', datestr( max(x) ) );

axes(handles.plot_axes);
yl = get(handles.plot_axes, 'YLim' ) ;

hold on;
handles.rmDateLine1 = line( [ min(x) min(x) ], yl, 'LineStyle', ':', 'Color', 'g' );
handles.rmDateLine2 = line( [ max(x) max(x) ], yl, 'LineStyle', ':', 'Color', 'g' );

guidata(hObject,handles);


% --- Executes on selection change in firTypeMenu.
function firTypeMenu_Callback(hObject, eventdata, handles)
% hObject    handle to firTypeMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns firTypeMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from firTypeMenu


% --- Executes during object creation, after setting all properties.
function firTypeMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to firTypeMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function firWidthEdit_Callback(hObject, eventdata, handles)
% hObject    handle to firWidthEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of firWidthEdit as text
%        str2double(get(hObject,'String')) returns contents of firWidthEdit as a double


% --- Executes during object creation, after setting all properties.
function firWidthEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to firWidthEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit15_Callback(hObject, eventdata, handles)
% hObject    handle to edit15 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit15 as text
%        str2double(get(hObject,'String')) returns contents of edit15 as a double


% --- Executes during object creation, after setting all properties.
function edit15_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit15 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in firButton.
function firButton_Callback(hObject, eventdata, handles)
% hObject    handle to firButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

s = get( handles.firTypeMenu, 'String' );
idx = get( handles.firTypeMenu, 'Value' );
w = str2double( get( handles.firWidthEdit, 'String') );
if ~iscell(s),
    s = { s };
end

opts = struct( 'type', s{idx} , 'width', w );
[ dummy, new_corr ] = tscorr_firfilter( handles.xx_date, handles.xx_corr(:,end), opts );

% add a new correction
handles.xx_corr(:,end+1) = new_corr;
handles.corr_table{end+1} = sprintf( 'fir_filter, type=%s, width=%d', s{idx}, w );    
print_corrtable( handles.corr_table );
makePlot(hObject, eventdata, handles);
guidata(hObject,handles);


% --------------------------------------------------------------------
function ExportMenu_Callback(hObject, eventdata, handles)
% hObject    handle to ExportMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function exportOVLMat_Callback(hObject, eventdata, handles)
% hObject    handle to exportOVLMat (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

s = get( handles.polMenu, 'String' );
idx = get( handles.polMenu, 'Value' );

res = ovldb_exportdlg( ...
    'Name', 'Export OVLdb...', ...
    'PolName', lower(s{idx}), ...
    'Station', get( handles.stationEdit, 'String' ) );

if ~isfield( res, 'Save' ) || ~res.Save 
    fprintf( 'Cancelled...\n' );
    return;
end

fprintf( 'Saving to %s...\n', res.fileName );
write_aqdata( res.fileName, handles.xx_date, handles.xx_corr(:,end), ...
    'Format', 'ovldb', ...
    'Timestamp', 'after', ...
    'stationIndex', str2double( get( handles.stationIdxEdit, 'String' ) ), ...
    'ovlAggr', res.aggr, ...
    'selHours', res.selHours, 'hr1', res.hr1, 'hr2', res.hr2 );

uiwait(msgbox(sprintf( '%s is saved...', res.fileName ),'modal'));


% --------------------------------------------------------------------
function exportLIBOVITOAscii_Callback(hObject, eventdata, handles)
% hObject    handle to exportLIBOVITOAscii (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[filename, pathname] = uiputfile( ...
{'*.txt;*.asc;*.dat','Text files (*.txt,*.asc,*.dat)'; 
 '*.*',  'All Files (*.*)'},...
 'Save as LIBOVITO ascii file...', ...
 handles.fname );

output_file = fullfile( pathname, filename );

fprintf( 'Saving to %s...\n', output_file );
write_aqdata( output_file, handles.xx_date, handles.xx_corr(:,end),  ...
    'Format', 'libovito', ...
    'Timestamp', 'after' );



function stationIdxEdit_Callback(hObject, eventdata, handles)
% hObject    handle to stationIdxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of stationIdxEdit as text
%        str2double(get(hObject,'String')) returns contents of stationIdxEdit as a double


% --- Executes during object creation, after setting all properties.
function stationIdxEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to stationIdxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function transfParEdit_Callback(hObject, eventdata, handles)
% hObject    handle to transfParEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of transfParEdit as text
%        str2double(get(hObject,'String')) returns contents of transfParEdit as a double


% --- Executes during object creation, after setting all properties.
function transfParEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to transfParEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
