function coords = parse_coords( str )
C = my_strsplit( strtrim(str), ';' );
coords = zeros(numel(C), 2);
for i=1:numel(C)
   cc = my_strsplit( C{i}, ',' );
   coords(i,1) = str2double(cc(1));
   coords(i,2) = str2double(cc(2));
end
