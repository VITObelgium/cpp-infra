
% earlier matlab releases dont contain this function
function C = my_strsplit( str, delim )
remain = str;
C = cell(1,1);
i=1;
while ~isempty(remain)
   [s, remain] = strtok(remain, delim);
   C{i} = s;
   i = i+1;
end