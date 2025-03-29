<?php
// Set proper headers
header("Content-Type: text/html");

// Get the request method
$method = $_SERVER['REQUEST_METHOD'];
echo "<html><body>";
echo "<h1>File Upload Test</h1>";

if ($method == "POST") {
    // Read raw POST data
    $input = file_get_contents('php://input');
    $size = strlen($input);

    echo "<p>Received data size: $size bytes</p>";
    echo "<p>First 50 bytes: " . htmlspecialchars(substr($input, 0, 50)) . "...</p>";

    // Display POST variables
    if (isset($_POST)) {
        echo "<p>POST variables:</p>";
        echo "<pre>";
        print_r($_POST);
        echo "</pre>";
    }
} else {
    echo "<p>This script expects POST requests.</p>";
}

echo "</body></html>";
?>