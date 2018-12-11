function varargout = configure_stations(varargin)
% CONFIGURE_STATIONS M-file for configure_stations.fig
%      CONFIGURE_STATIONS, by itself, creates a new CONFIGURE_STATIONS or raises the existing
%      singleton*.
%
%      H = CONFIGURE_STATIONS returns the handle to a new CONFIGURE_STATIONS or the handle to
%      the existing singleton*.
%
%      CONFIGURE_STATIONS('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in CONFIGURE_STATIONS.M with the given input arguments.
%
%      CONFIGURE_STATIONS('Property','Value',...) creates a new CONFIGURE_STATIONS or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before configure_stations_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to configure_stations_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help configure_stations

% Last Modified by GUIDE v2.5 18-Feb-2011 15:04:49

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @configure_stations_OpeningFcn, ...
                   'gui_OutputFcn',  @configure_stations_OutputFcn, ...
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


% --- Executes just before configure_stations is made visible.
function configure_stations_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to configure_stations (see VARARGIN)

% display 
st_id    = varargin{1};
st_info  = varargin{2};
ign_list = varargin{3};

data      = cell( size( st_id, 1 ), 3 );
data(:,1) = num2cell( st_info(:,1) );
data(:,2) = st_id;
data(:,3) = num2cell( 1 );

for i=1:length(ign_list)
    idx = find( [data{:,1}] == ign_list(i) );
    data{idx,3} = 0;
end

set( handles.uitable1, 'Data', data );

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes configure_stations wait for user response (see UIRESUME)
uiwait( handles.figure1 );


% --- Outputs from this function are returned to the command line.
function varargout = configure_stations_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% get the list of non-0 in column 3 of the uitable2
data  = get( handles.uitable1, 'data' );

idx   = find( cell2mat(data(:,3)) == 0 );
st_id = cell2mat( data(:,1) );

% Get default command line output from handles structure
varargout{1} = st_id(idx);

delete( handles.figure1 );


% --- Executes on button press in pushbutton1.
function pushbutton1_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

figure1_CloseRequestFcn( hObject, eventdata, handles );


% --- Executes when user attempts to close figure1.
function figure1_CloseRequestFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: delete(hObject) closes the figure
uiresume( handles.figure1 );



