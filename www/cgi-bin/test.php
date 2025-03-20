#!/usr/bin/php-cgi
<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<html><body>";
echo "<h1>PHP CGI Test</h1>";
echo "<p>Server Time: " . date("Y-m-d H:i:s") . "</p>";

// Handle the URL parameter
if (isset($_GET['name'])) {
    echo "<p>Hello, " . htmlspecialchars($_GET['name']) . "!</p>";
}

echo "</body></html>";
?>