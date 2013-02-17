#ifndef _WEBPAGE_H_
#define _WEBPAGE_H_

#define INDEX_HTML  "<html>"\
                    "<head>"\
                    "<title>Webpage configuration: password - Nuri</title>"\
                    "</head>"\
                    "<body>"\
                      "<fieldset sytle='border-color:#E2DED6;border-width:1px;border-style:Solid'>"\
                        "<legend style='color:#333;font-size:1.2em;font-weight:bold'>"\
                    "Please input your password"\
                      "</legend>"\
                      "<center>"\
                        "<form action='password.pl' name='frmPwd' method='post'>"\
                          "</br>"\
                          "<label for='txtPwd'>Password: </label><input type='password' name='pwd' id='txtPwd' size='32' maxlength='32'/></br></br>"\
                          "<input type='submit' value='Confirm'/>"\
                       "</form>"\
                      "</center>"\
                    "</fieldset>"\
                    "</body>"\
                    "</html>"

#define CONFIG_HTML "<html>"\
        "<head>"\
        "<title>Serial to Ethernet G/W module - nuri</title>"\
        "<script language='javascript'>"\
        "function WidgetCallback(oJson)"\
        "{"\
            "var obj=oJson;"\
            "var txt=obj['txt'];"\
            "var sel=obj['sel'];"\
            "var oTxt=document.getElementsByTagName('input');"\
            "var cnt=0;"\
            "for(var i=0;i<oTxt.length;i++)"\
            "{"\
                "if(oTxt[i].type!='text') cnt++;"\
                "else "\
                  "oTxt[i].value=txt[i-cnt]['v'];"\
            "}"\
            "var oDhcp=document.getElementsByName('dhcp');"\
            "for(var i=0; i<oDhcp.length; i++)"\
            "{"\
                "if(i==obj['dhcp'])"\
                "{"\
                    "oDhcp[i].checked=true;"\
                    "break;"\
                "}"\
            "}"\
            "var oRs232=document.getElementsByName('rs232');"\
            "for(var i=0; i<oRs232.length; i++)"\
            "{"\
                "if(i==obj['rs232'])"\
                "{"\
                    "oRs232[i].checked=true;"\
                    "break;"\
                "}"\
            "}"\
            "var oMode=document.getElementsByName('mode');"\
            "for(var i=0; i<oMode.length; i++)"\
            "{"\
               "if(i==obj['mode'])"\
                "{"\
                    "oMode[i].checked=true;"\
                    "break;"\
                "}"\
            "}"\
            "var oSel=document.getElementsByTagName('select');"\
            "for(var i=0;i<oSel.length;i++)"\
            "{"\
                "for(var j=0;j<oSel[i].options.length;j++)"\
                "{"\
                    "if(j==sel[i]['v'])"\
                    "{"\
                        "oSel[i].options[j].selected=true;"\
                        "break;"\
                    "}"\
                "}"\
            "}"\
            "var oChkLoc=document.getElementById('chkLoc');"\
            "if(obj['loc']==1)"\
                "oChkLoc.checked=true;"\
            "else "\
                "oChkLoc.checked=false;"\
            "var oChkKa=document.getElementById('chkKa');"\
            "if(obj['ka']==1)"\
                "oChkKa.checked=true;"\
            "else "\
                "oChkKa.checked=false;"\
            "var oChkNtp=document.getElementById('chkNtp');"\
            "if(obj['ntp']==1)"\
                "oChkNtp.checked=true;"\
            "else "\
                "oChkNtp.checked=false;"\
        "}"\
        "function changepwd()"\
        "{"\
        "var obj=document.getElementById('pwd');"\
        "if(obj)"\
         "{"\
              "if(obj.style.display=='none')"\
                  "obj.style.display='block';"\
              "else "\
                  "obj.style.display='none';"\
          "}"\
        "}"\
        "</script>"\
        "<style>"\
        "body{"\
                "font-family: Verdana, Geneva, Arial, Helvetica, sans-serif;"\
                "font-size:12px;"\
                "color:#444;"\
                "line-height:150%;"\
                "padding:0;"\
        "}"\
        "fieldset{"\
        "border-color:#E2DED6;"\
        "border-width:1px;"\
        "border-style:Solid;"\
        "margin-bottom:10px;"\
        "}"\
        "legend{"\
        "color:#333;"\
        "font-size:1.2em;"\
        "font-weight:bold;"\
        "}"\
        "fieldset div{margin:5px 15px;}"\
        ".main{padding:5px;width:960px;margin:0px auto;}"\
        "#net p{clear:left;}"\
        "#net label{float:left;margin-left:-5px;padding-top:3px;text-align:left;width:130px;}"\
        "#timer label{float:left;margin-left:-5px;padding-top:3px;text-align:left;width:130px;}"\
        "#pwd label{float:left;margin-left:-5px;padding-top:3px;text-align:left;width:160px;}"\
        ".mr5{margin-right:5px;}"\
        ".red{color:red}"\
        "</style>"\
        "</head>"\
        "<body>"\
        "<form action='config.pl' id='frm' method='post'>"\
        "<div class='main'>"\
        "<fieldset>"\
        "<legend>Device Information</legend>"\
        "<div>"\
        "<p><label for='txtname'>Device name:</label><input type='text' size='20' id='txtname' name='devicename' value='' /><label class='red'>(16 bytes in Max.)</label></p>"\
        "<p><label class='mr5'>Serial number: </label><input type='text' size='20' disabled='disabled' value='' /></p>"\
        "<p><label class='mr5'>Device Mac address: </label><input type='text' size='20' disabled='disabled' value='' /></p>"\
        "</div>"\
        "</fieldset>"\
        "<fieldset>"\
            "<legend>"\
                "Device Network Settings"\
            "</legend>"\
            "<div>"\
                "<p>"\
                "<input type='radio' name='dhcp' id='rdStatic' value='static' /><label for='rdStatic'>Using the follow IP address</label>"\
                "<input type='radio' name='dhcp' id='rdDhcp' value='dhcp' /><label for='rdDhcp'>Get IP address from DHCP server</label>"\
                "</p>"\
                "<div id='net'>"\
                "<p>"\
                "<label for='txtSip'>Device IP address:</label><input id='txtSip' name='sip' type='text' size='20' value='' />"\
                "<input style='margin-left:5px;' id='txtlport' name='localport' type='text'  size='10' value='' />"\
                "</p>"\
                "<p>"\
                "<label for='txtGw'>Gateway:</label><input id='txtGw' name='gwip' type='text' size='20' value='' />"\
                "</p>"\
                "<p>"\
                "<label for='txtSub'>Subnet mask:</label><input id='txtSub' name='sn' type='text' size='20' value='' />"\
                "</p>"\
                "<p>"\
                "<label for='txtDns'>DNS server:</label><input id='txtDns' name='dns' type='text'  size='20' value='' />"\
                "</p>"\
                "</div>"\
            "</div>"\
        "</fieldset>"\
        "<fieldset>"\
            "<legend>"\
                "COM Port Settings"\
            "</legend>"\
            "<div>"\
            "<p>"\
                "<input type='radio' id='rd232' name='rs232' value = 'rs232'><label for='rd232'>RS232 Device</label>"\
                "<input type='radio' id='rd485' name='rs232' value = 'rs485'><label for='rd485'>RS485 Device</label>"\
                "<label class='red'>(This option is defined by H/W)</label>"\
            "</p>"\
            "<p>"\
                "<label for='bdrate'>Baudrate:</label>"\
                "<select id='bdrate' name='baudrate'>"\
                    "<option>1200</option>"\
                    "<option>2400</option>"\
                    "<option>4800</option>"\
                    "<option>9600</option>"\
                    "<option>19200</option>"\
                    "<option>38400</option>"\
                    "<option>57600</option>"\
                    "<option>115200</option>"\
                "</select>"\
                "<label for='selDb'>Data bit:</label>"\
                "<select id='selDb' name='databits'>"\
                "<option>7</option>"\
                "<option>8</option>"\
                "</select>"\
                "<label for='selParity'>Parity:</label>"\
                "<select id='selParity' name='parity'>"\
                "<option>NONE</option>"\
                "<option>ODD</option>"\
                "<option>EVEN</option>"\
                "</select>"\
                "<label for='selStop'>Stop bit:</label>"\
                "<select id='selStop' name='stopbits'>"\
                "<option>1</option>"\
                "</select>"\
                "<label for='selFlow'>Flow control:</label>"\
                "<select id='selFlow' name='flowcontrol'>"\
                "<option>NONE</option>"\
                "<option>CTS/RTS</option>"\
                "</select>"\
            "</p>"\
            "</div>"\
        "</fieldset>"\
        "<fieldset>"\
            "<legend>"\
                "Working mode Setting"\
            "</legend>"\
            "<div>"\
            "<p>"\
            "<input type='radio' id='rdsvr' name='mode' value = 'TCPS'><label for='rdsvr'>TCP Server</label>"\
            "<input type='radio' id='rdclt' name='mode' value = 'TCPC'><label for='rdclt'>TCP Client</label>"\
            "<input type='radio' id='rdmix' name='mode' value = 'TCPM'><label for='rdmix'>TCP Mixed</label>"\
            "<input type='radio' id='rdudp' name='mode' value = 'UDP'><label for='rdudp'>UDP Mode</label>"\
            "</p>"\
            "<p><label for='txtrhost' class='mr5'>Remote host:</label><input id='txtrhost' name='remotehost' type='text' size='32' value='' class='mr5' /><label class='mr5' for='txtrport'>port number:</label><input id='txtrport' name='remoteport' type='text' size='10' value='' class='mr5' /></p>"\
            "</div>"\
        "</fieldset>"\
        "<fieldset>"\
            "<legend>"\
                "Locating Server Settings"\
            "</legend>"\
            "<div>"\
            "<p><input id='chkLoc' type='checkbox' name='enlocating' value='enable' /><label for='chkLoc'>Enable locating server</label></p>"\
            "<p>"\
            "<label for='txtlocip'>Server IP address: </label><input id='txtlocip' name='serveripname' type='text' size='20' value='' />"\
            "<label for='txtlocport' style='margin-left:10px;'>Port number: </label><input id='txtlocport' name='serverportname' type='text'  size='20' value='' />"\
            "</p>"\
            "<p>"\
              "<input type='checkbox' id='chkKa' name='enkeepalive' value='enable' />Enable send Locationg message every <input type='text' name='keepinterval' value='' size='10' /> seconds.<!-- for--><input type='text' style='display:none;' name='keepinterval2' value='' size='10' /> <!--times-->"\
            "</p>"\
            "</div>"\
        "</fieldset>"\
        "<fieldset>"\
            "<legend>"\
                "Date&Time Settings"\
            "</legend>"\
             "<div>"\
             "<p>"\
             "<input id='chkNtp' name='ntpenable' type='checkbox' value='enabled' /><label for='chkNtp' style='margin-right:20px'>Enable NTP </label>"\
             "<label for='txtntp'></label>NTP server IP address: <input id='txtntp' type='text' name='ntpserver' value='' size='20'/>"\
             "</p>"\
             "<p>"\
             "<label for='seltz'>Time zone:</label>"\
              "<select id='seltz' name='timezone'>"\
                "<option>00)UTC-12:00 Baker Island, Howland Island (both uninhabited)</option>"\
                        "<option>01)UTC-11:00 American Samoa, Samoa</option>"\
                        "<option>02)UTC-10:00 (Summer)French Polynesia (most), United States (Aleutian Islands, Hawaii)</option>"\
                        "<option>03)UTC-09:30 Marquesas Islands </option>"\
                        "<option>04)UTC-09:00 Gambier Islands;(Summer)United States (most of Alaska)</option>"\
                        "<option>05)UTC-08:00 (Summer)Canada (most of British Columbia), Mexico (Baja California)</option>"\
                        "<option>06)UTC-08:00 United States (California, most of Nevada, most of Oregon, Washington (state))</option>"\
                        "<option>07)UTC-07:00 Mexico (Sonora), United States (Arizona); (Summer)Canada (Alberta)</option>"\
                        "<option>08)UTC-07:00 Mexico (Chihuahua), United States (Colorado)</option>"\
                        "<option>09)UTC-06:00 Costa Rica, El Salvador, Ecuador (Galapagos Islands), Guatemala, Honduras</option>"\
                        "<option>10)UTC-06:00 Mexico (most), Nicaragua;(Summer)Canada (Manitoba, Saskatchewan), United States (Illinois, most of Texas)</option>"\
                        "<option>11)UTC-05:00 Colombia, Cuba, Ecuador (continental), Haiti, Jamaica, Panama, Peru</option>"\
                        "<option>12)UTC-05:00 (Summer)Canada (most of Ontario, most of Quebec)</option>"\
                        "<option>13)UTC-05:00 United States (most of Florida, Georgia, Massachusetts, most of Michigan, New York, North Carolina, Ohio, Washington D.C.)</option>"\
                        "<option>14)UTC-04:30 Venezuela</option>"\
                        "<option>15)UTC-04:00 Bolivia, Brazil (Amazonas), Chile (continental), Dominican Republic, Canada (Nova Scotia), Paraguay,</option>"\
                        "<option>16)UTC-04:00 Puerto Rico, Trinidad and Tobago</option>"\
                        "<option>17)UTC-03:30 Canada (Newfoundland)</option>"\
                        "<option>18)UTC-03:00 Argentina; (Summer) Brazil (Brasilia, Rio de Janeiro, Sao Paulo), most of Greenland, Uruguay</option>"\
                        "<option>19)UTC-02:00 Brazil (Fernando de Noronha), South Georgia and the South Sandwich Islands</option>"\
                        "<option>20)UTC-01:00 Portugal (Azores), Cape Verde</option>"\
                        "<option>21)UTC&#177;00:00 Cote d'Ivoire, Faroe Islands, Ghana, Iceland, Senegal; (Summer) Ireland, Portugal (continental and Madeira)</option>"\
                        "<option>22)UTC&#177;00:00 Spain (Canary Islands), Morocco, United Kingdom</option>"\
                        "<option>23)UTC+01:00 Angola, Cameroon, Nigeria, Tunisia; (Summer)Albania, Algeria, Austria, Belgium, Bosnia and Herzegovina,</option>"\
                        "<option>24)UTC+01:00 Spain (continental), Croatia, Czech Republic, Denmark, Germany, Hungary, Italy, Kinshasa, Kosovo, </option>"\
                        "<option>25)UTC+01:00 Macedonia, France (metropolitan), the Netherlands, Norway, Poland, Serbia, Slovakia, Slovenia, Sweden, Switzerland</option>"\
                        "<option>26)UTC+02:00 Libya, Egypt, Malawi, Mozambique, South Africa, Zambia, Zimbabwe, (Summer)Bulgaria, Cyprus, Estonia, </option>"\
                        "<option>27)UTC+02:00 Finland, Greece, Israel, Jordan, Latvia, Lebanon, Lithuania, Moldova, Palestine, Romania, Syria, Turkey, Ukraine</option>"\
                        "<option>28)UTC+03:00 Belarus, Djibouti, Eritrea, Ethiopia, Iraq, Kenya, Madagascar, Russia (Kaliningrad Oblast), Saudi Arabia, </option>"\
                        "<option>29)UTC+03:00 South Sudan, Sudan, Somalia, South Sudan, Tanzania, Uganda, Yemen</option>"\
                        "<option>30)UTC+03:30 (Summer)Iran</option>"\
                        "<option>31)UTC+04:00 Armenia, Azerbaijan, Georgia, Mauritius, Oman, Russia (European), Seychelles, United Arab Emirates</option>"\
                        "<option>32)UTC+04:30 Afghanistan</option>"\
                        "<option>33)UTC+05:00 Kazakhstan (West), Maldives, Pakistan, Uzbekistan</option>"\
                        "<option>34)UTC+05:30 India, Sri Lanka</option>"\
                        "<option>35)UTC+05:45 Nepal</option>"\
                        "<option>36)UTC+06:00 Kazakhstan (most), Bangladesh, Russia (Ural: Sverdlovsk Oblast, Chelyabinsk Oblast)</option>"\
                        "<option>37)UTC+06:30 Cocos Islands, Myanmar</option>"\
                        "<option>38)UTC+07:00 Jakarta, Russia (Novosibirsk Oblast), Thailand, Vietnam</option>"\
                        "<option>39)UTC+08:00 China, Hong Kong, Russia (Krasnoyarsk Krai), Malaysia, Philippines, Singapore, Taiwan, most of Mongolia, Western Australia</option>"\
                        "<option>40)UTC+09:00 East Timor, Russia (Irkutsk Oblast), Japan, North Korea, South Korea</option>"\
                        "<option>41)UTC+09:30 Australia (Northern Territory);(Summer)Australia (South Australia))</option>"\
                        "<option>42)UTC+10:00 Russia (Zabaykalsky Krai); (Summer)Australia (New South Wales, Queensland, Tasmania, Victoria)</option>"\
                        "<option>43)UTC+10:30 Lord Howe Island</option>"\
                        "<option>44)UTC+11:00 New Caledonia, Russia (Primorsky Krai), Solomon Islands</option>"\
                        "<option>45)UTC+11:30 Norfolk Island</option>"\
                        "<option>46)UTC+12:00 Fiji, Russia (Kamchatka Krai);(Summer)New Zealand</option>"\
                        "<option>47)UTC+12:45 (Summer)New Zealand</option>"\
                        "<option>48)UTC+13:00 Tonga</option>"\
                        "<option>49)UTC+14:00 Kiribati (Line Islands)</option>"\
               "</select>"\
               "</p>"\
               "<p>"\
               "<label for='txtDatetime'>Using manually input datetime:</label><input type='text' id='txtDatetime' name='datetime' size='32' value='' />"\
               "<label class='red'>(Format: yyyy-mm-dd hh:mm:ss)</label>"\
               "</p>"\
             "</div>"\
        "</fieldset>"\
        "<fieldset>"\
            "<legend>"\
            "Timer options"\
            "</legend>"\
            "<div id='timer'>"\
            "<p><label for='txtnagle'>Nagle wait time: </label><input id='txtnagle' type='text' name='nagle' value='' size='8' /> <span class='red'>(milisecond)</span></p>"\
            "<p><label for='txtinact'>Inactivity time: </label><input id='txtinact' type='text' name='inact' value='' size='8' /> <span class='red'>(second)</span></p>"\
            "<p><label for='txtrecon'>Reconnection time: </label><input id='txtrecon' type='text' name='recon' value='' size='8' /> <span class='red'>(milisecond)</span></p>"\
            "</div>"\
        "</fieldset>"\
        "<fieldset>"\
            "<legend>"\
                "Device password"\
            "</legend>"\
             "<div>"\
             "<a href='javascript:;' onclick='javascript:changepwd();'>Change password</a>"\
             "</div>"\
             "<div id='pwd' style='display:none;'>"\
             "<p><label for='txtPwd'>Input old password:</label><input id='txtOldPwd' type='password' name='password' value='' size='32' /></p>"\
             "<p><label for='txtPwd'>Input new password:</label><input id='Password1' type='password' name='newpwd' value='' size='32' /></p>"\
             "<p><label for='txtPwd'>Confirm new password:</label><input id='Password2' type='password' name='newpwd2' value='' size='32' /></p>"\
             "</div>"\
        "</fieldset>"\
        "<p><input type='submit' style='margin-right:10px;' value='Save Setting' /><a href='log_msg.bin' style='margin-right:10px;' target='_blank'>Download Log Message</a><a href='log_msg.txt' target='_blank'>View Log Message</a></p>"\
        "</div>"\
        "</form>"\
        "<script type='text/javascript' src='widget.pl'></script>"\
        "</body>"\
        "</html>"

#endif