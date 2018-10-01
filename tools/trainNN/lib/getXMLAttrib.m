function x = getXMLAttrib( xml, name )
x = xml.Attributes( strcmp( {xml.Attributes.Name}, name) ).Value;
