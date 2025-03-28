<?php
header("Content-Type: text/html");
echo "<html><body>";
echo "<h1>Hello, CGI World!</h1>";
if (isset($_POST['name'])) {
    echo "<p>Name: " . htmlspecialchars($_POST['name']) . "</p>";
} else {
    echo "<p>No name provided in POST request.</p>";
}
echo "</body></html>";
?>
