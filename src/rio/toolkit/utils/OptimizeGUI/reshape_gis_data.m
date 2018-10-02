function rio_class_dist = reshape_gis_data( gis_data, rio_class_agg )


%----------------------------
%-- Reshape GIS data:
%-- combine CORINE classes 
%-- to reduce dimensions...
%----------------------------
num_rio_classes = length( rio_class_agg );
rio_class_dist  = zeros( size( gis_data, 1), num_rio_classes );

for i=1:num_rio_classes
    rio_class_dist(:,i) = sum( gis_data( :, rio_class_agg{i} ) , 2 );
end

return
