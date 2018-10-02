function [ opts, args ] = getopt_cast( options, optchar, varargin )
%GETOPT_CAST
%
% Quick and dirty routine to parse command line arguments in matlab and
% to be able to give posix-style command line arguments to a compiled
% matlab program. It's not completely compatible probably, but it more
% or less does the trick. And has some nifty type casting as well :-). It 
% basically parses a varargin such as : 
%
% { '--start', '250', '--help', '--stop', '2012-01-01', 'pm10', 'hello' }
%
% So with support for parameter arguments and switches, which don't require
% a value. The optchar should be a string with which to precede command line
% argument names, e.g. '--' as in :
%
%  program --fname foo --option bar arg1 arg2
%
% The opts structure should contain a list of possible optional arguments,
% with default values. Argument names should not have the optchar.
%
% opts = [ 
%  struct( 'name', 'start', 'default', 5, 'cast', @(x)str2double(x)), ...
%  struct( 'name', 'stop',  'default', 'test', 'cast', @(x)(datenum(x))), ...
%  struct( 'name', 'help',  'default', false,  'cast', NaN ) ...
% ];
%
% So basically it is an array of structures, each having a name, default
% and cast funtion handle to cast the text argument to a value in the 
% structure. The routine returns a structure with the fieldnames as given
% from the name member in the opts array. If you want a simple switch, than
% put in NaN or something else, but not a function handle in the cast
% routine. 
% 
% If the final value should stay a text string, than just put something
% like @(x)(x) as function handle. 
% 
% Returns modified structure with options from the varargin, options are
% given by the optchar, which is something like '-' or '--'. The args
% output argument contains a cell array of the leftover arguments, which 
% were not preceded by the optchar. In the above example this would be 
% { 'arg1', 'arg2' }
% 
% Author: Bino Maiheu, based on some matlab central code found at 
%         http://www.mathworks.com/matlabcentral/newsreader/view_thread/56523

% Get list of the names
prop_names = {options.name}; 

% init output strcture and set defaults
opts = struct();
args = {};
for k=1:length(options)
    opts.(options(k).name) = options(k).default;
end

TargetField  = [];
CastFunction = @(x)(x);
v = varargin{:};

for ii=1:length(v)
  arg = v{ii};
  args = {v{ii:end}};
  if isempty(TargetField)
    if ~ischar(arg) || ~strncmp(arg,optchar,length(optchar));                
        return;
    end
    
    f = find(strcmp(prop_names, strrep(arg,optchar,'')));
    if isempty(f)
      error('%s ',['invalid property ''',arg,'''; must be oneof:'],prop_names{:});
    end
        TargetField  = strrep(arg,optchar,'');
    if isa( options(f).cast, 'function_handle' )         
        CastFunction = options(f).cast;
    else
       % this option does not expect an argument, just a boolean switch, 
       % which is now turned on and we leave TargetField etc empty 
       % for the next loop
       opts.(TargetField) = true;    
       TargetField  = '';
    end
  else    
    opts.(TargetField) = CastFunction( arg );
    TargetField  = '';
    CastFunction = @(x)(x);
  end
end
if ~isempty(TargetField)
  error('Property names and values must be specified in pairs.');
end
