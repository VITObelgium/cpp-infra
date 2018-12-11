function varargout = rioDataAnalyser(varargin)
% RIODATAANALYSER MATLAB code for rioDataAnalyser.fig
%      RIODATAANALYSER, by itself, creates a new RIODATAANALYSER or raises the existing
%      singleton*.
%
%      H = RIODATAANALYSER returns the handle to a new RIODATAANALYSER or the handle to
%      the existing singleton*.
%
%      RIODATAANALYSER('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in RIODATAANALYSER.M with the given input arguments.
%
%      RIODATAANALYSER('Property','Value',...) creates a new RIODATAANALYSER or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before rioDataAnalyser_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to rioDataAnalyser_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help rioDataAnalyser

% Last Modified by GUIDE v2.5 07-Oct-2013 11:43:41

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @rioDataAnalyser_OpeningFcn, ...
                   'gui_OutputFcn',  @rioDataAnalyser_OutputFcn, ...
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


% --- Executes just before rioDataAnalyser is made visible.
function rioDataAnalyser_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to rioDataAnalyser (see VARARGIN)

% Choose default command line output for rioDataAnalyser
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes rioDataAnalyser wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = rioDataAnalyser_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



function edit_dbqFileName_Callback(hObject, eventdata, handles)
% hObject    handle to edit_dbqFileName (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit_dbqFileName as text
%        str2double(get(hObject,'String')) returns contents of edit_dbqFileName as a double


% --- Executes during object creation, after setting all properties.
function edit_dbqFileName_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit_dbqFileName (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton_loadFileName.
function pushbutton_loadFileName_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_loadFileName (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[filename, pathname] = uigetfile({ '*.dat;*.dbq;*.txt', 'Text files (*.dat,*.dbq,*.txt)';...
  '*.*', 'All files...'}, 'Load dbq file...', 'MultiSelect', 'off' );
if ~( isequal(filename,0) || isequal(pathname,0) )  
  set( handles.edit_dbqFileName, 'String', fullfile(pathname,filename) );
end



% --- Executes on selection change in popupmenu1.
function popupmenu1_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns popupmenu1 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu1


% --- Executes during object creation, after setting all properties.
function popupmenu1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton2.
function pushbutton2_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Getting config info from the handles
i      = get( handles.popupmenu1, 'Value' );
tmp    = get( handles.popupmenu1, 'String' );
pol_xx = tmp{i};
fname  = get( handles.edit_dbqFileName, 'String' );

% Here we go...
fprintf( 'Parsing %s data from %s\n', pol_xx,  fname );
st_list = rioDataAnalyser_GetTimeWindows( pol_xx, fname );

% Get output file...
[filename, pathname] = uiputfile( {'*.txt', 'Text file (*.txt)' }, ...
  'Save station summary as...');
 if ~( isequal(filename,0) || isequal(pathname,0) )  
   ofileID = fopen( fullfile( pathname, filename ), 'wt' );
 else
   fprintf( 'Cancelled by user...\n' );
   return;
 end

fn = fieldnames( st_list );
% match for station types ?
st_fname = get( handles.edit3, 'String');
if isempty( st_fname  )
  fprintf( 'No station file given, skipping this bit....\n' );
  for i=1:length( fn )
    st_id = regexprep(fn{i}, '^ST', '');
    fprintf( ofileID, '%d\t%s\t%s\t%s\n', i, st_id, ...
      datestr( st_list.(fn{i}).start_date), datestr( st_list.(fn{i}).end_date ) ); 
  end
  return;
end

fprintf( 'Reading station information from %s\n', st_fname );
fileID = fopen( st_fname, 'r' );
A = textscan( fileID, '%s%d%d%d%s', 'Delimiter', ';', 'Headerlines', 1 );
fclose(fileID);
for i=1:length( fn )
  st_id = regexprep(fn{i}, '^ST', '');
  idx = find( strcmp( st_id, A{1} ) );
  if ~isempty(idx)
    st_list.(fn{i}).x    = A{3}(idx);
    st_list.(fn{i}).y    = A{4}(idx);
    st_list.(fn{i}).type = A{2}(idx);
    st_list.(fn{i}).desc = A{5}{idx};       
  else
    st_list.(fn{i}).x    = NaN;
    st_list.(fn{i}).y    = NaN;
    st_list.(fn{i}).type = 0;
    st_list.(fn{i}).desc = 'N/A';   
  end
  fprintf( ofileID, '%d\t%s\t%d\t%d\t%d\t%s\t%s\t%s\n', i, st_id, ...
      st_list.(fn{i}).x, st_list.(fn{i}).y, st_list.(fn{i}).type,  ...
      st_list.(fn{i}).desc, ...
    datestr( st_list.(fn{i}).start_date), datestr( st_list.(fn{i}).end_date ) );
end

fclose(ofileID);


% --- Executes on button press in pushbutton3.
function pushbutton3_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[filename, pathname] = uigetfile({ '*.dat;*.dbq;*.txt', 'Text files (*.dat,*.dbq,*.txt)';...
  '*.*', 'All files...'}, 'Load dbq file...', 'MultiSelect', 'off' );
if ~( isequal(filename,0) || isequal(pathname,0) )  
  set( handles.edit3, 'String', fullfile(pathname,filename) );
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
