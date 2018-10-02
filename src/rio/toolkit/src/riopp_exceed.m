%riopp_exceed Computes norm exceedances
%
%
% [noe_grid, noe_st ] = riopp_avg( C, obs, avg_err, eu_limit, eu_max_exceed )
%
%
% Output matrices contain colmnvectors : 
%
% noe_grid : <noe> <noe_low> <noe_upp> <noe_exceed_prob> [ <da_noe> <da_noe_low> <da_noe_upp> <da_noe_exceed_prob>]
% noe_st   : <noe> [ <da_noe> ]
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ noe_grid, noe_st ] = riopp_exceed( C, obs, avg_err, eu_limit, eu_max_exceed )

if length( eu_limit ) > 1
    noe_grid = zeros( size(C,1), 8 );
    noe_st   = zeros( size(obs,1), 2 );
else
    noe_grid = zeros( size(C,1), 4 );
    noe_st   = zeros( size(obs,1), 1 );
end

% Grid exceedances...
%-- Method according to Bruce Denby...
if eu_limit(1) > 0    
    noe_st(:,1)   = sum( obs > eu_limit(1), 2 );
    
    noe_grid(:,1) = sum( C   > eu_limit(1), 2 );
    noe_grid(:,2) = sum( bsxfun( @minus, C, avg_err) > eu_limit(1), 2 );
    noe_grid(:,3) = sum( bsxfun( @plus,  C, avg_err) > eu_limit(1), 2 );

    %-- Exceedance probability...
    sig = .5 * ( noe_grid(:,3) - noe_grid(:,2) );        
    noe_grid(:,4) = 100. * ( 1. - rio_normcdf( eu_max_exceed(1), noe_grid(:,1), sig ) );
    noe_grid( isnan( noe_grid(:,4) ),4 ) = 0.;
else
    noe_st(:,1)     = -1;
    noe_grid(:,1:4) = -1;
end

%-- Calculate daily aggregations...
if length( eu_limit ) > 1
    if eu_limit(2) > 0
        C_da   = riopp_aggregate( 'nanmean', C );
        obs_da = riopp_aggregate( 'nanmean', obs );
        
        noe_st(:,2)   = sum( obs_da > eu_limit(2), 2 );
        
        noe_grid(:,5) = sum( C_da   > eu_limit(2), 2 );
        noe_grid(:,6) = sum( bsxfun( @minus, C_da, avg_err ) > eu_limit(2), 2 );
        noe_grid(:,7) = sum( bsxfun( @plus,  C_da, avg_err ) > eu_limit(2), 2 );
        
        sig = .5 * ( noe_grid(:,7) - noe_grid(:,6) );
        noe_grid(:,8) = 100. * ( 1. - rio_normcdf( eu_max_exceed(2), noe_grid(:,5), sig ) );
        noe_grid( isnan( noe_grid(:,8) ),8 ) = 0.;
    else
        noe_st(:,2)     = -1;
        noe_grid(:,5:8) = -1;
    end
end