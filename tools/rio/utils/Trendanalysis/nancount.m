function y = nancount(x,dim)
% FORMAT: Y = NANCOUNT(X,DIM)
% 
%    Counts the number of non-NaN values 
%

if isempty(x)
	y = 0;
	return
end

if nargin < 2
	dim = min(find(size(x)~=1));
	if isempty(dim)
		dim = 1;
	end
end
% Replace NaNs with zeros.
nans = isnan(x);
x(isnan(x)) = 0; 
y = size(x,dim) - sum(nans,dim);
