<html>
<head>
<title></title>
<script language=javascript>
j=0;
function func(){
	document.getElementById('delay').innerText=' '+j+' ';
	j--;
	setTimeout('func()',1000);
	if(j<0)
		location.href='led.htm';
}
</script>
</head>

<body onload='func()'>
Please wait for a while, the configuration page will be loaded automatically in<span style='color:red;' id='delay'></span> seconds.
</body>
</html>

<!--
<? SetValue(LED1) ?>
<? SetValue(LED2) ?>
-->
