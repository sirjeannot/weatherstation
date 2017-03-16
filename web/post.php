<?php
header("Content-type: text/html");
header("Cache-Control: no-cache, must-revalidate"); // HTTP/1.1
$avg=$_POST['avg'];
$gust=$_POST['gust'];
//if (is_numeric($avg) && is_numeric($gust))

	$timestamp=date("H:i");
	$filename=date("Ymd");
	$handle = fopen("log/{$filename}.log", "a+");
	fwrite($handle,"{$timestamp},{$avg},{$gust}\n");
	fclose($handle);
	echo "0";

//else
//	echo "1";
?>