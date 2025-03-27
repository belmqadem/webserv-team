<?php
header("Content-Type: text/html");
sleep(5);
echo "<html><body>Hello, CGI World!</body></html>";
echo $_REQUEST['name'];
?>
