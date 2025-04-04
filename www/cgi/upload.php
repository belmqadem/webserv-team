<?php
header("Content-Type: text/html");

if (isset($_FILES['file'])) {
    echo "Processing file: " . $_FILES['file']['name'] . "<br>";
    
    $uploaddir = '../uploads/';
    $uploadfile = $uploaddir . basename($_FILES['file']['name']);
    
    if (move_uploaded_file($_FILES['file']['tmp_name'], $uploadfile)) {
        echo "File uploaded successfully!";
        http_response_code(200);
    } else {
        echo "Upload failed!";
        http_response_code(500);
    }
} else {
    echo "No input file specified.";
    http_response_code(404);
}
?>