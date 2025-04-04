<?php
// Debug script: save as www/cgi/debug.php
header("Content-Type: text/plain");

echo "FILES array: ";
var_dump($_FILES);

echo "\n\nCONTENT_TYPE: " . $_SERVER['CONTENT_TYPE'];
echo "\nINPUT LENGTH: " . strlen(file_get_contents("php://input")) . " bytes";

http_response_code(200); // Force 200 response
?>