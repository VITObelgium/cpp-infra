function gamma = semivariance( a, c, n, h, model )

switch model
    case 'spherical'
        gamma = zeros(size(h));
        gamma(h>a)  = c + n;
        gamma(h<=a) = c*((3*h(h<=a)./(2*a))-1/2*(h(h<=a)./a).^3) + n;        
    otherwise
        error( 'not supported' );
end
