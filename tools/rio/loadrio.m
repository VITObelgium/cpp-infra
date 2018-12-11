% librio MATLAB startup file
% Bino Maiheu (c) 2011 VITO
if ~ismcc && ~isdeployed
	[path, filename ] = fileparts( mfilename('fullpath') );

	addpath([path filesep 'src']);
	addpath([path filesep 'utils']);
	addpath([path filesep 'utils' filesep 'OptimizeGUI']);

	% Returns some feedback
	v = rio_version;
	disp ( sprintf( 'RIO library, version %d.%d (c) VITO 2004-2011', v.major, v.minor ) );

	% Cleanup 
	clear filename;
	clear path;
	clear v;
end