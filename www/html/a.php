<?php
echo "Query String: " . $_SERVER['QUERY_STRING'] . "<br>";
echo "Name from GET: " . ($_GET["name"] ?? "Not Set") . "<br>";
print_r($_GET);
?>
