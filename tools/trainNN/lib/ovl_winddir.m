%-------------------------------------------
%-- calculate day-average of winddirection
%-------------------------------------------

function [ success, x, y ] = ovl_winddir( array, missing, success )

index=find(array~=missing);

if (isempty(index))
    success=0;
    x=1;y=1;
else
    x=0;y=0;
    array=array(index);
    for n=1:max(size(array))        % vectorsum of all direction-vectors
        x=x + cos(2*pi*array(n)/360);     %-- Attention: was minus sign before!!!
        y=y + sin(2*pi*array(n)/360);     %-- Attention: was minus sign before!!!
    end
    norm=sqrt(x^2+y^2);
    x=x/norm;
    y=y/norm;
end