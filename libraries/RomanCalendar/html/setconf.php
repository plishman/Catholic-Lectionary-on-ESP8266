<?php
error_log("\n\n---settings.json test harness---");
$target_path = dirname(__FILE__);

$json = json_encode($_GET);
error_log($json);

$filename = $target_path . "/settings.json";
error_log("filename is " . $filename);

$file = fopen($filename, "w");
fwrite($file, $json);
fclose($file);
//header("Location: /setconf.htm");
print("Message from setconf.php: Complete");
exit();
?>