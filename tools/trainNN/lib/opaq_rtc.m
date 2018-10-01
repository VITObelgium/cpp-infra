% OPAQ_RTC perform the real time corrections for the pure NN forecasts
%
% rtc_value = opaq_rtc( xx_date, xx_value, fc_value, fc_hor, varargin)
%
% Optional parameters :
%
%  'hc_range', ndays  : range for the hindcast in number of days
%                       default is 20
%
%  'rtc_mode', id     : a real time correction mode id, implemented are:
%          0   - no real time correction
%          1   - mean hind cast error is forecast error
%          2   - weighted mean hindcast error
%
%  'rtc_param', <value> : a parameter for mode 2 (default 1)
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function rtc_value = opaq_rtc( xx_date, xx_value, fc_value, fc_hor, varargin)

p = inputParser;
p.CaseSensitive = true;
p.addParameter( 'hc_range', 20, @isnumeric ); % hind carst range
p.addParameter( 'rtc_mode',  0, @(x) assert( isnumeric(x) && isscalar(x) ) );
p.addParameter( 'rtc_param', 1, @isnumeric );
p.parse( varargin{:} );

% -- compute hindcast database
hc_db = ovl_hcerr( fc_hor, xx_date, xx_value, fc_value, p.Results.hc_range );
%hc_db = opaq_hcerr( xx_date, xx_value, fc_value, fc_hor, p.Results.hc_range );

% -- compute the forecast errors according to the mode
fc_err = opaq_fcerr( hc_db, p.Results.rtc_mode, p.Results.rtc_param );

% -- introduce the trheshold : below no corrections are applied
rtc_value = fc_value - fc_err;
