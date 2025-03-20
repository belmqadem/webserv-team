#!/usr/bin/php-cgi
<?php
echo "Content-Type: text/html\r\n\r\n";  // Required CGI header
echo "<html><body><h1>PHP CGI Test</h1>";
echo "<p>Server Time: " . date("Y-m-d H:i:s") . "</p>";
echo "</body></html>";
?>
