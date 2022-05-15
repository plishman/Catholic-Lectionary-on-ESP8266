<?php

error_log("\n\n---FILE UPLOADER---");
error_log(print_r($_FILES, true));
$target_path = dirname(__FILE__);

$file = reset($_FILES);

if ($file) {
    $target_path = $target_path . "/" . basename($file['name']);

    error_log("Source=" .        $file['name']);
    error_log("type=" .          $file['type']);
    error_log("tmp_name=" .      $file['tmp_name']);
    error_log("Error=" .         $file['error']);
    error_log("Target path=" .   $target_path);
    error_log("Size=" .          $file['size']);    
    
    if(move_uploaded_file($file['tmp_name'], $target_path)) {
        error_log("The file ".  basename( $file['name'])." has been uploaded");
    } else{
        error_log("There was an error uploading the file, please try again!");
    }
}
?>