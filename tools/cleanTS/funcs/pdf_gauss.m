%PDF_GAUSS Gaussian probability density function
%
% Bino Maiheu
function y = pdf_gauss( x, mu, sigma )
 y = exp(-0.5*((x-mu)./sigma ).^2)./(sigma*sqrt(2*pi));