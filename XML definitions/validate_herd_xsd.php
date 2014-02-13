<?php
$xdoc = new DomDocument;
$xmlfile = 'sample_herds.xml';
$xmlschema = 'herds.xsd';
//Load the xml document in the DOMDocument object
$xdoc->Load($xmlfile);
//Validate the XML file against the schema
if ($xdoc->schemaValidate($xmlschema)) {
print "$xmlfile is valid.\n";
} else {
print "$xmlfile is INVALID!.\n";
}
?>
