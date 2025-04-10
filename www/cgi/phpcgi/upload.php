<?php
// Show all errors
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

header('Content-Type: text/plain');

echo "PHP Upload Test\n";
echo "==============\n\n";

// First check what might be wrong with the file upload
echo "File Upload Debug:\n";
echo "Field names in FILES array: " . implode(", ", array_keys($_FILES)) . "\n\n";

// Check for common file field names
$file_fields = array('file', '@file', 'userfile', 'upload', 'uploadfile');
echo "Checking for common field names:\n";
foreach ($file_fields as $field) {
    echo "  Field '$field': " . (isset($_FILES[$field]) ? "Present" : "Not present") . "\n";
}
echo "\n";

// Check input method
echo "REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD'] . "\n";
echo "CONTENT_TYPE: " . $_SERVER['CONTENT_TYPE'] . "\n";
echo "CONTENT_LENGTH: " . $_SERVER['CONTENT_LENGTH'] . "\n\n";

// Check if we have raw POST data
$input = file_get_contents('php://input');
echo "Raw input size: " . strlen($input) . " bytes\n";
if (strlen($input) > 0) {
    echo "First 100 bytes: " . substr($input, 0, 100) . "\n\n";
}

// Check if PHP parsed the form correctly
echo "POST array size: " . count($_POST) . "\n";
foreach ($_POST as $key => $value) {
    echo "  $key => " . substr($value, 0, 30) . "...\n";
}

// Check FILES array
echo "\nFILES array size: " . count($_FILES) . "\n";
foreach ($_FILES as $key => $file) {
    echo "  $key => [name: " . $file['name'] . 
         ", size: " . $file['size'] . 
         ", error: " . $file['error'] . 
         ", tmp_name: " . $file['tmp_name'] . "]\n";
    
    // Check error code
    if ($file['error'] > 0) {
        echo "    Error code: " . $file['error'] . " - ";
        switch ($file['error']) {
            case UPLOAD_ERR_INI_SIZE:
                echo "File exceeds upload_max_filesize";
                break;
            case UPLOAD_ERR_FORM_SIZE:
                echo "File exceeds MAX_FILE_SIZE in form";
                break;
            case UPLOAD_ERR_PARTIAL:
                echo "File was only partially uploaded";
                break;
            case UPLOAD_ERR_NO_FILE:
                echo "No file was uploaded";
                break;
            case UPLOAD_ERR_NO_TMP_DIR:
                echo "Missing temporary folder";
                break;
            case UPLOAD_ERR_CANT_WRITE:
                echo "Failed to write file to disk";
                break;
            case UPLOAD_ERR_EXTENSION:
                echo "Upload stopped by extension";
                break;
            default:
                echo "Unknown error";
        }
        echo "\n";
    } else {
        // Try to save the file if uploaded successfully
        $upload_dir = "../../uploads/";
        if (!file_exists($upload_dir)) {
            mkdir($upload_dir, 0755, true);
            echo "    Created upload directory: $upload_dir\n";
        }
        
        $target_file = $upload_dir . basename($file['name']);
        if (move_uploaded_file($file['tmp_name'], $target_file)) {
            echo "    File successfully saved to: $target_file\n";
        } else {
            echo "    Error saving file. Check permissions.\n";
        }
    }
}

// Check tmp directory
echo "\nTemp dir access:\n";
echo "upload_tmp_dir = " . ini_get('upload_tmp_dir') . "\n";
if (!empty(ini_get('upload_tmp_dir'))) {
    echo "Directory exists: " . (is_dir(ini_get('upload_tmp_dir')) ? "Yes" : "No") . "\n";
    echo "Directory writable: " . (is_writable(ini_get('upload_tmp_dir')) ? "Yes" : "No") . "\n";
} else {
    echo "Default system temp dir: " . sys_get_temp_dir() . "\n";
    echo "Directory exists: " . (is_dir(sys_get_temp_dir()) ? "Yes" : "No") . "\n";
    echo "Directory writable: " . (is_writable(sys_get_temp_dir()) ? "Yes" : "No") . "\n";
}

header("Status: 201");
