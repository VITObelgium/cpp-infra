% OPAQ_ISBETTER
%
% bool = OPAQ_isbetter( best_stat, new_stat, criterium )
%
% This function determines which statistic structure is better in terms of
% validation that the other, depending on the selected criterium.
% Statistics structures have to be returend by the ovl_validate function.
%
% Criteriums can be simple names of the statistics fields, in which case
% the routine wil auto-select whether to favour higher or lower values
% of the requested statistic... For the implemented fieldnames :
%
% SI        : successindex
% FCF       : fraction of correct forecast events
% FRF       : fraction of realised forecast events
% SFN       : skill of forecasting non-events
% FFA       : Fraction of false alarm  (NEW DEFINITION!!!!)
% FI        :  Failure Index (new)
% R2        : ... square ( explained variance )
% RMSE      : root mean square error
% BIAS      : bias
%
%
% Changelog
% 2015-12-14 : - adjusted from ovl_isbetter
%              - included bugfix for bias : take absolute values !!
%              - using lower case names to match validstats
%
% Author  : Bino Maiheu
% Contact : bino.maiheu@vito.be

function  [ bool ] = opaq_isbetter( best, new, crit )

bool = false;

% If the current best structure is empty, the new structure is always
% better
if isempty( fieldnames( best ) )
    bool = true;
    return;
end
    
switch crit
    case{ 'ffa', 'fi', 'rmse' } % the lower the better  
        if new.( crit ) < best.( crit )
            bool = true;
            return;
        end
        
    case { 'bias' } % for bias take absolute values...         
        if abs( new.( crit ) ) < abs( best.( crit ) )
            bool = true;
            return;
        end
        
    case { 'si', 'fcf', 'frf', 'sfn', 'r2' }
        % the higher the better
        if new.( crit ) > best.( crit )
            bool = true;
            return;
        end
        
    otherwise
        error( 'OPAQ:ValidationError', 'criterium %s is not available in statistics structure', crit );        
end