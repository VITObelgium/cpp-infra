function list = getTagAttribValues( tree, tagname, attribname )

cnfListItems = tree.getElementsByTagName( tagname );
list = {};
for k=0:cnfListItems.getLength-1
    cnfItem = cnfListItems.item(k);
    
    cnfAttribList = cnfItem.getAttributes;
    for i = 0:cnfAttribList.getLength-1
        attrib = cnfAttribList.item(i);
        if strcmp( char(attrib.getName), attribname );            
            list{end+1} = char(attrib.getValue);
            break;
        end        
    end
end
list = list(:);
