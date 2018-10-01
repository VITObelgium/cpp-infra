% OPAQ_GETMODELS Returns a list of the available models in the library
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function model_list = opaq_getmodels( varargin )

base_path = '.';
if nargin == 1
    base_path = varargin{1};
end

% get list of model files
lst = dir( fullfile( base_path, '+opaqmodels', '*.m' ) );

% remove the trailing suffix 
model_list = strrep( {lst.name}, '.m', '' );

% remove the abstract base class from the list
model_list( strcmp( model_list, 'opaq_model' ) ) = [];


    
    

