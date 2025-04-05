<?php
// Set status code explicitly
header("HTTP/1.1 200 OK");
header("Content-Type: text/html");

echo "<html><body>";
echo "<h1>Hello, CGI World!</h1>";
if (isset($_GET['client'])) {
    echo "<p>Client ID: " . htmlspecialchars($_GET['client']) . "</p>";
}
echo "<p>No name provided in POST request.</p>";
echo "</body></html>";
echo "<!-- Response after sleep -->";
?>