function [ data ] = reset_ai_weights( w, n_dim )

a = cell( 1 );
a{1} = w;
data = zeros( length(a{1}), 4*n_dim );
for i=1:n_dim
    data(:,4*i-3) = a{i}';
    data(:,4*i-2) = 1;
    data(:,4*i-1) = a{i}'-0.1*a{i}'; % lower bound, default range : 10 %
    data(:,4*i)   = a{i}'+0.1*a{i}'; % upper bound, default range : 10 %
end

