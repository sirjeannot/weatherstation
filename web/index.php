<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html  xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>Lagos Wind</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
<meta http-equiv="refresh" content="300"/>
<link rel="icon" type="image/x-icon" href="favicon.ico"/>
<style type="text/css">
	body {
		margin: 0px;
		padding: 0px;
	}
	#container {
		width : 1000px;
		height: 384px;
		margin: 8px auto;
	}
</style>
</head>




<body>

<?php
	$timestamp=date("H:i");
	if (!isset($_GET["date"]))
		$today=date("Ymd");
	else
		$today=$_GET["date"];
	$data1="";
	$data2="";
	if (file_exists("log/{$today}.log")) {
		$handle = fopen("log/{$today}.log", "r");
		while (!feof($handle)) {
			$line=fgets($handle);
  			$line = explode(',',$line);
			$temp=substr($line[0],3,2)/6*10;	//compute minutes in 100th of hour
			$temp=sprintf("%02d",round($temp));
			$line[0]=substr($line[0],0,2) . "." . $temp;
			for ( $k=0; $k<3; $k++) {
				if ($line[$k][0]=='0')
					$line[$k]=substr($line[$k],1);
			}
			$data1.="[" . $line[0] . ", " .  $line[1] . "],";
			$line[2]=substr($line[2],0,-1);	//remove \n for new line
			$data2.="[" . $line[0] . ", " .  $line[2] . "],";
		}
		fclose($handle);
		$data1=substr($data1,0,-9);
		$data2=substr($data2,0,-9);
		$today=date("Y-m-d");
	}
?>

<div align="center">Lagos Wind</div>

<div id="container"></div>
<script type="text/javascript" src="flotr2.min.js"></script>
<script type="text/javascript">
	(function () {
		var
			container = document.getElementById('container'),
			d1 = [],
			d2 = [<?php echo $data1; ?>],
			d3 = [<?php echo $data2; ?>],
			graph;
		function construct () {
			// Draw Graph
			graph = Flotr.draw(container, [ d1, {data: d2, label: 'average', lines: {fill: true}}, {data: d3, label: 'gust'} ], {
				subtitle: '<?php echo $today; ?>',
				xaxis: {
					min: 0,
					max: 24,
					//timeUnit: 'hour',
					//mode: 'time',
					noTicks: 24,
					tickDecimals: 0
        			},
				yaxis : {
					max: 20,
					min: 0,
					noTicks: 10,
					tickDecimals: 0
				},
				legend: {
            				position: 'nw'
        			}
			});
		}
		construct();
	})();
</script>

<br/>
<a href="index.php">today's stats</a><br/>
<a href=<?php $yesterday=date("Ymd",strtotime('-1 days')); echo "\"index.php?date={$yesterday}\"";?>>yesterday's stats</a><br/>
<a href="log/">all logs</a><br/>
last update : <?php echo "{$today} {$timestamp}"; ?><br/>
<a href="about.html">about</a><br/>
</body>
</html>