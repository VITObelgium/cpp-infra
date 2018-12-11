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