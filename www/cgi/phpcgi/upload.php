<?php
// www/cgi/upload.php
header("Content-Type: text/html");

$upload_dir = "../uploads/";

// Create directory if it doesn't exist
if (!file_exists($upload_dir)) {
    mkdir($upload_dir, 0755, true);
}

if ($_FILES && isset($_FILES['file'])) {
    $file = $_FILES['file'];
    
    if ($file['error'] === UPLOAD_ERR_OK) {
        $tmp_name = $file['tmp_name'];
        $name = "upload_" . time() . "_" . basename($file['name']);
        $upload_path = $upload_dir . $name;
        
        // Move the temporary file PHP created
        if (move_uploaded_file($tmp_name, $upload_path)) {
            echo "File uploaded successfully!<br>";
            echo "Saved as: " . htmlspecialchars($name);
            http_response_code(201);
        } else {
            echo "Error saving file";
            http_response_code(500);
        }
    } else {
        // Various upload error conditions
        echo "Upload error: " . $file['error'];
        http_response_code(400);
    }
} else {
    echo "No file uploaded";
    http_response_code(400);
}
?>