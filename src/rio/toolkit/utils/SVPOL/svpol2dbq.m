function svpol2dbq( fname, pol, outputfile )

[ st_name, xx_date, xx_vals48 ] = read_svpol( fname, pol );

xx_vals48 = 100.*xx_vals48;

xx_vals48( xx_vals48 < 0 ) = NaN;
xx_val48_1 = xx_vals48(:,1:2:end);
xx_val48_2 = xx_vals48(:,2:2:end);
xx_val_1h  = .5*( xx_val48_1 + xx_val48_2 ); % make use of NaN -> if one value is NaN -> average is NaN
xx_val_1h( isnan(xx_val_1h) ) = -9999;   % reset missing values

xx_val_m1 = max( xx_val_1h, [], 2 );
xx_val_da = eu_mean( xx_val_1h );
xx_val_m8 = -9999 * ones( size(xx_val_m1 ) );
xx_val_m1( isnan( xx_val_m1 ) ) = -9999;
xx_val_da( isnan( xx_val_m1 ) ) = -9999;



fid = fopen( outputfile, 'wt' );
for k=1:size(xx_date)
    export = [ round(xx_val_m1(k)) round(xx_val_m8(k)) round(xx_val_da(k)) round(xx_val_1h(k,:)) ];
  
  
    fprintf( fid, '%6s %6s %6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n', ...
        st_name{k}, datestr( xx_date(k), 'yyyymmdd' ),  export );
    
  
  %fprintf( fid, '%6s %6s %10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f%10.3f\n', ...
  %    st_list{k}, datestr( t_days.xx_date(k), 'yyyymmdd' ), export );
end
fclose(fid);




function avg = eu_mean( x )
avg = -9999*ones(size(x,1),1);
for k=1:size(x,1)
    arr = x(k,:);
    if numel( find( arr < 0 ) ) > 6 % need at least 75 % valid hourly values
        avg(k) = -9999;
    else        
        avg(k) = mean( arr( arr > 0 ) );
    end
end