<html>
<head>
<title></title>
<script language=javascript>
j=10;
function func(){
	document.getElementById('delay').innerText=' '+j+' ';
	j--;
	setTimeout('func()',1000);
	if(j==0)
		location.href='ipconfig.htm';
}
</script>
</head>

<body onload='func()'>
Please wait for a while, the configuration page will be loaded automatically in<span style='color:red;' id='delay'></span> seconds.
</body>
</html>

<? SetValue(IPAddr=IP) ?>
<? SetValue(GWAddr=GW) ?>
<? SetValue(Subnet=SUB) ?>
<? SetValue(DNSAddr=DNS) ?>
