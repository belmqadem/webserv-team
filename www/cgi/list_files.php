<?php
$uploadDir = '../delete/';
$files = array_diff(scandir($uploadDir), array('.', '..'));
header('Content-Type: application/json');
echo json_encode(array_values($files));
?>
