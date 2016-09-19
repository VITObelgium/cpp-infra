%TSCORR_HARDLIM Hard limits timeseries correction
%
% Usage: 
%   [ xx_date, xx_vals ] = tscorr_hardlim( xx_date, xx_vals, lo, hi )
%
% Bino Maiheu, (c) VITO 2014

function [ xx_date_new, xx_vals_new ] = tscorr_hardlim( xx_date, xx_vals, lo, hi )

xx_date_new = xx_date;
xx_vals_new = xx_vals;
xx_vals_new( xx_vals_new < lo | xx_vals_new > hi ) = NaN;

