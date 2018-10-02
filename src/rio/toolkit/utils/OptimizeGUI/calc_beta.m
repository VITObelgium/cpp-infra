
function [ beta ] = calc_beta( a, class_dist )
 
 denom = class_dist * a';
 nom   = sum( class_dist, 2 );

 beta = log( 1. +  denom./nom )'; 

end
