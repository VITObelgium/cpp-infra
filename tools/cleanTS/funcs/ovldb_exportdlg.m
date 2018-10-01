function varargout = ovldb_exportdlg(varargin)
% OVLDB_EXPORTDLG MATLAB code for ovldb_exportdlg.fig
%      OVLDB_EXPORTDLG by itself, creates a new OVLDB_EXPORTDLG or raises the
%      existing singleton*.
%
%      H = OVLDB_EXPORTDLG returns the handle to a new OVLDB_EXPORTDLG or the handle to
%      the existing singleton*.
%
%      OVLDB_EXPORTDLG('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in OVLDB_EXPORTDLG.M with the given input arguments.
%
%      OVLDB_EXPORTDLG('Property','Value',...) creates a new OVLDB_EXPORTDLG or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before ovldb_exportdlg_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to ovldb_exportdlg_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help ovldb_exportdlg

% Last Modified by GUIDE v2.5 28-Mar-2014 00:32:05

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @ovldb_exportdlg_OpeningFcn, ...
                   'gui_OutputFcn',  @ovldb_exportdlg_OutputFcn, ...
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

% --- Executes just before ovldb_exportdlg is made visible.
function ovldb_exportdlg_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to ovldb_exportdlg (see VARARGIN)

% Choose default command line output for ovldb_exportdlg
handles.output = struct( 'Save', false );
handles.pol_name = 'xx';
handles.station_name = 'ST0000';


% Insert custom Title and Text if specified by the user
% Hint: when choosing keywords, be sure they are not easily confused 
% with existing figure properties.  See the output of set(figure) for
% a list of figure properties.
if(nargin > 3)
    for index = 1:2:(nargin-3),
        if nargin-3==index, break, end
        switch lower(varargin{index})
            case 'title'
                set(hObject, 'Name', varargin{index+1});
            case 'polname'
                handles.pol_name = varargin{index+1};
            case 'station'
                handles.station_name = varargin{index+1};
        end
    end
end


set( handles.fnameEdit, 'String', ...
    sprintf( '%s_%s.mat', handles.pol_name, handles.station_name ) );

% Update handles structure
guidata(hObject, handles);

% Make the GUI modal
set(handles.figure1,'WindowStyle','modal')

% UIWAIT makes ovldb_exportdlg wait for user response (see UIRESUME)
uiwait(handles.figure1);

% --- Outputs from this function are returned to the command line.
function varargout = ovldb_exportdlg_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

% The figure can be deleted now
delete(handles.figure1);

% --- Executes on button press in saveButton.
function saveButton_Callback(hObject, eventdata, handles)
% hObject    handle to saveButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

handles.output = struct( 'Save', true );
handles.output.fileName = get( handles.fnameEdit, 'String' );
handles.output.selHours = ( get( handles.selHoursBox, 'Value' ) == 1);
handles.output.hr1 = str2double( get( handles.hr1Edit, 'String' ) );
handles.output.hr2 = str2double( get( handles.hr2Edit, 'String' ) );

s = get( handles.aggrMenu, 'String' );
i = get( handles.aggrMenu, 'Value' );
handles.output.aggr = s{i};

% Update handles structure
guidata(hObject, handles);

% Use UIRESUME instead of delete because the OutputFcn needs
% to get the updated handles structure.
uiresume(handles.figure1);

% --- Executes on button press in cancelButton.
function cancelButton_Callback(hObject, eventdata, handles)
% hObject    handle to cancelButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

handles.output = struct( 'Save', false );

% Update handles structure
guidata(hObject, handles);

% Use UIRESUME instead of delete because the OutputFcn needs
% to get the updated handles structure.
uiresume(handles.figure1);


% --- Executes when user attempts to close figure1.
function figure1_CloseRequestFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if isequal(get(hObject, 'waitstatus'), 'waiting')
    % The GUI is still in UIWAIT, us UIRESUME
    uiresume(hObject);
else
    % The GUI is no longer waiting, just close it
    delete(hObject);
end


% --- Executes on key press over figure1 with no controls selected.
function figure1_KeyPressFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Check for "enter" or "escape"
if isequal(get(hObject,'CurrentKey'),'escape')
    % User said no by hitting escape
    handles.output = 'No';
    
    % Update handles structure
    guidata(hObject, handles);
    
    uiresume(handles.figure1);
end    
    
if isequal(get(hObject,'CurrentKey'),'return')
    uiresume(handles.figure1);
end    



function fnameEdit_Callback(hObject, eventdata, handles)
% hObject    handle to fnameEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of fnameEdit as text
%        str2double(get(hObject,'String')) returns contents of fnameEdit as a double


% --- Executes during object creation, after setting all properties.
function fnameEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to fnameEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton4.
function pushbutton4_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[filename, pathname] = uiputfile( ...
    {'*.mat','Matlab files (*.mat)';
    '*.*',  'All Files (*.*)'},...
    'Save as OVL db file...', ...
 sprintf( '%s_%s.mat', handles.pol_name, handles.station_name ) );
if isequal(filename,0) || isequal(pathname,0)
    fprintf( 'Cancelled...\n' );
    return;
end

set( handles.fnameEdit, 'String', fullfile( pathname, filename ) );
guidata( hObject, handles );


% --- Executes on selection change in aggrMenu.
function aggrMenu_Callback(hObject, eventdata, handles)
% hObject    handle to aggrMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns aggrMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from aggrMenu


% --- Executes during object creation, after setting all properties.
function aggrMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to aggrMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function hr1Edit_Callback(hObject, eventdata, handles)
% hObject    handle to hr1Edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of hr1Edit as text
%        str2double(get(hObject,'String')) returns contents of hr1Edit as a double


% --- Executes during object creation, after setting all properties.
function hr1Edit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to hr1Edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in selHoursBox.
function selHoursBox_Callback(hObject, eventdata, handles)
% hObject    handle to selHoursBox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of selHoursBox



function hr2Edit_Callback(hObject, eventdata, handles)
% hObject    handle to hr2Edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of hr2Edit as text
%        str2double(get(hObject,'String')) returns contents of hr2Edit as a double


% --- Executes during object creation, after setting all properties.
function hr2Edit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to hr2Edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
