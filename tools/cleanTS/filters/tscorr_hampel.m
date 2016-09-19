function [ xx_date_new, xx_vals_new ] = tscorr_hampel( xx_date, xx_vals, logtr, DX, T )

X = xx_date;
if logtr
    Y = log(1+xx_vals);
else
    Y = xx_vals;
end
if DX < 1
    DX = 3*median(xx_date(2:end)-xx_date(1:end-1));
else
    DX = DX*median(xx_date(2:end)-xx_date(1:end-1));
end

[YY,I,Y0,LB,UB] = hampel(xx_date,Y,DX,T);

figure( 'Name', sprintf( 'HAMPEL FILTER, LOGTRANS=%d, DX=%f.2, T=%f.2', logtr, DX, T ), ...
    'Position', [ 50 50 1200 400 ] );
plot(xx_date, Y, 'b.'); hold on;      % Original Data
plot(xx_date, YY, 'r');               % Hampel Filtered Data
plot(xx_date, Y0, 'b--');             % Nominal Data
plot(xx_date, LB, 'r--');             % Lower Bounds on Hampel Filter
plot(xx_date, UB, 'r--');             % Upper Bounds on Hampel Filter
plot(xx_date(I), Y(I), 'ks');         % Identified Outliers
grid on;
datetick( 'keeplimits' );
set(zoom(gcf),'ActionPostCallback', @setDateTicks);
set(pan(gcf),'ActionPostCallback', @setDateTicks);


xx_date_new = xx_date;
if logtr    
    xx_vals_new = exp(YY)-1;
else 
    xx_vals_new = YY;
end


function setDateTicks(figureHandle,eventObjectHandle)
% Resetting x axis to automatic tick mark generation 
set(eventObjectHandle.Axes,'XTickMode','auto')
% using automaticallly generate date ticks
datetick(eventObjectHandle.Axes,'x','keeplimits')