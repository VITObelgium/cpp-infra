addpath 'D:\Projects\N7862\2013\trunk\src';
addpath 'D:\Matlab\MatlabToolkit\Mapping\m_map';

clear variables;
close all;

tmp = importdata( 'h:\devel\opaq\plugins\ipolsimple\test\grid.out');

gr_x   = tmp(:,1);
gr_y   = tmp(:,2);
gr_val = tmp(:,3);

tmp = importdata( 'h:\devel\opaq\plugins\ipolsimple\test\stations.out');
st_x  = tmp(:,1);
st_y  = tmp(:,2);
st_val = tmp(:,3);

rio_map(gr_val);
hold on;
scatter(st_x,st_y);

n_gr = numel(gr_x);
n_st = numel(st_x);



%% check

r = zeros(n_gr,n_st);
for i=1:n_gr
    for j=1:n_st
        r(i,j) = sqrt( (gr_x(i)-st_x(j)).^2 + (gr_y(i)-st_y(j)).^2 );        
    end
end
p = 1;
w = 1./r.^p;
w = bsxfun(@rdivide,w,sum(w,2));

gr_val_matlab = w*st_val;
