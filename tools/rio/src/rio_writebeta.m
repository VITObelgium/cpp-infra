function [ beta ] = rio_writebeta( pol, cnf_file, class_dist_file, grid_file )
 
xml   = xmlread( cnf_file );
polEl = getTagByAttribValue( xml, 'pollutant', 'name', pol );


ai = [];
cd = {};


if ~isempty(polEl) && polEl.hasChildNodes
    clElements = polEl.getElementsByTagName( 'rio_class' );
    for i=0:clElements.getLength-1
        clEl = clElements.item(i);        
        attlist = clEl.getAttributes;
        have_weight = false;
        for j=1:attlist.getLength
            if strcmp( attlist.item(j-1).getName, 'weight' )
                value = str2double( attlist.item(j-1).getValue );
                ai(end+1) = value;
                have_weight = true;
            end
        end
        if ~have_weight
            error( 'No weight specified for class' );
        end
        
        % get the class distributions
        cd{end+1} = str2num(clEl.getChildNodes.item(0).getData);
    end
else
    error( 'pollutant not found...' );
end

% reading class distribution...
clc = importdata( class_dist_file, ';', 1 );
clc_name = clc.textdata(1,4:49);
clc_frac = clc.data(:,4:49);

% now reshape gis data
nc = length(cd);
fprintf( 'We have %d classes...\n', nc );
rio_frac = zeros( size(clc_frac,1), nc );
for i=1:nc
    fprintf( 'Aggregating class %d...\n', i );
    
    idx = zeros(1,length(cd{i}));
    for j=1:length(cd{i})
        ii = find( strcmp( clc_name, num2str(cd{i}(j)) ) );
        if ~any(ii)
            error( 'cannot find the requested class in the class_dist file...' );
        else
            idx(j) = ii;
        end
    end
    
    rio_frac( :, i )  = sum( clc_frac( :, idx ), 2 );
        
end

% check

% calculate beta
denom = rio_frac * ai';
nom   = sum( rio_frac, 2 );
beta  = log( 1. +  denom./nom )';

% write output
grd = importdata( grid_file );
export = [ grd.data(:,1:3) beta' ];

fid = fopen( sprintf( '%s_beta.txt', pol ), 'wt'  );
fprintf( fid, 'ID\tX\tY\tBETA\n' );
fprintf( fid, '%d\t%f\t%f\t%f\n', export'  );
fclose(fid);



function el = getTagByAttribValue( tree, tagname, attribname, attribvalue )

cnfListItems = tree.getElementsByTagName( tagname );
for k=0:cnfListItems.getLength-1
    cnfItem = cnfListItems.item(k);
    
    cnfAttribList = cnfItem.getAttributes;
    for i = 0:cnfAttribList.getLength-1
        attrib = cnfAttribList.item(i);
        if strcmp( char(attrib.getName), attribname );            
            if strcmp( char(attrib.getValue), attribvalue )
                el = cnfItem;
                return;
            end            
        end        
    end
end

el = [];