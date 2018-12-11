%RIO_VERSION
% Returns a structure with the version information for the rio toolkit
% 
% v = rio_version
%
% The structure contains the major and minor fields.
%
% See also rio_init
%
% RIO (c) VITO/IRCEL 2004-2016 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function v = rio_version
v = struct( 'major', 2, 'minor', 0 );