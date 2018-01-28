// home.js
//  Web front-end script for mz80rpi
//  2017-2018 Nibbles Lab.

var host = location.hostname;
var port = location.port;
var ws = new WebSocket("ws://"+host+":"+port+"/websocket");
var chk_attr = [];

//
// Event definition
//
document.addEventListener('DOMContentLoaded', function() {
    // receive WebSocket
    ws.addEventListener('message', function (event) {
        //console.log("received(ws):" + event.data);
        var jsdata = JSON.parse(event.data);
        for ( var key in jsdata ) {
            switch(key) {
                case "progress":
                    var progress = document.querySelector('.percent');
                    progress.style.width = jsdata.progress + '%';
                    break;
                case "motor":
                    var progress = document.querySelector('.percent');
                    if ( jsdata.motor == "on" ) {
                        progress.style.backgroundColor = '#99ccff';
                    } else if ( jsdata.motor == "off" ) {
                        progress.style.backgroundColor = '#808080';
                    }
                    break;
                case "led":
                    var led = document.querySelector('#led');
                    if ( jsdata.led == "green" ) {
                        led.style.backgroundColor = '#00ff00';
                    } else if ( jsdata.led == "red" ) {
                        led.style.backgroundColor = '#ff0000';
                    } else if ( jsdata.led == "none") {
                        led.style.backgroundColor = '#ffffff';
                    }
                    break;
                case "pcg":
                    if ( jsdata.pcg == "ON" ) {
                        document.getElementById('pcg1').checked = true;
                    } else {
                        document.getElementById('pcg0').checked = true;
                    }
                    break;
                default:
                    break;
            }
        }
    }, false);

    // Re-load
    document.getElementById('reload').addEventListener('click', function(e) {
        location.reload(true);
    }, false);
    // Reset Z80
    document.getElementById('zrst').addEventListener('click', function(e) {
        if (!window.confirm('Are you shure to reset CPU?')) {
            e.preventDefault();
        } else {
            aj_send({reset:"cpu"});
        }
    }, false);
    // PCG OFF
    document.getElementById('pcg0').addEventListener('click', function(e) {
        aj_send({pcg:"off"});
    }, false);
    // PCG ON
    document.getElementById('pcg1').addEventListener('click', function(e) {
        aj_send({pcg:"on"});
    }, false);
    // play CMT
    document.getElementById('play').addEventListener('click', function(e) {
        aj_send({cmt:"play"});
    }, false);
    // stop CMT
    document.getElementById('stop').addEventListener('click', function(e) {
        aj_send({cmt:"stop"});
    }, false);
    // eject CMT
    document.getElementById('eject').addEventListener('click', function(e) {
        aj_send({cmt:"eject"});
        var text = document.getElementById('casset');
        text.innerHTML = "Empty";
    }, false);
    // disp selected MZT info
    document.getElementById('imgname').addEventListener('change', function(e) {
        aj_send({mztinfo:document.getElementById('imgname').value});
    }, false);
    // MZT data set
    document.getElementById('setcmt').addEventListener('click', function(e) {
        aj_send({cmt:document.getElementById('imgname').value});
        var text = document.getElementById('casset');
        var obj = document.getElementById('imgname');
        var index = obj.selectedIndex;
        text.innerHTML = obj.options[index].text;
    }, false);
    // OBJ for MZT
    document.getElementById('obj').addEventListener('click', function(e) {
        aj_send({dblist:"obj"});
    }, false);
    // BTX(BASIC Text) for MZT
    document.getElementById('btx').addEventListener('click', function(e) {
        aj_send({dblist:"btx"});
    }, false);
    // BSD(BASIC Sequential data) for MZT
    document.getElementById('bsd').addEventListener('click', function(e) {
        aj_send({dblist:"bsd"});
    }, false);
    // Other data for MZT
    document.getElementById('oth').addEventListener('click', function(e) {
        aj_send({dblist:"oth"});
    }, false);
    // Input 1 line
    document.getElementById('cr').addEventListener('click', function(e) {
        aj_send({getl:document.getElementById('getl').value+'\x0d'});
        document.getElementById('getl').value = "";
    }, false);
    document.getElementById('getl').addEventListener('keypress', function(e) {
        if (e.keyCode == 13) {
            aj_send({getl:document.getElementById('getl').value});
            document.getElementById('getl').value = "";
        }
    }, false);
    document.getElementById('load_cmd').addEventListener('click', function(e) {
        document.getElementById('getl').value = 'LOAD';
    }, false);
    document.getElementById('run_cmd').addEventListener('click', function(e) {
        document.getElementById('getl').value = 'RUN';
    }, false);
    document.getElementById('goto_cmd').addEventListener('click', function(e) {
        document.getElementById('getl').value = 'GOTO\$';
    }, false);
    // Simple operation
    document.getElementById('break').addEventListener('click', function(e) {
        ws_send({getl:'\x03'});
    }, false);
    document.getElementById('clr').addEventListener('click', function(e) {
        ws_send({getl:'\x16'});
    }, false);
    document.getElementById('home').addEventListener('click', function(e) {
        ws_send({getl:'\x15'});
    }, false);
    document.getElementById('down').addEventListener('click', function(e) {
        ws_send({getl:'\x11'});
    }, false);
    document.getElementById('up').addEventListener('click', function(e) {
        ws_send({getl:'\x12'});
    }, false);
    document.getElementById('right').addEventListener('click', function(e) {
        ws_send({getl:'\x13'});
    }, false);
    document.getElementById('left').addEventListener('click', function(e) {
        ws_send({getl:'\x14'});
    }, false);
    document.getElementById('inst').addEventListener('click', function(e) {
        ws_send({getl:'\x18'});
    }, false);
    document.getElementById('del').addEventListener('click', function(e) {
        ws_send({getl:'\x17'});
    }, false);
    // reboot Linux
    document.getElementById('reboot').addEventListener('click', function(e) {
        if (!window.confirm('Are you shure to REBOOT system?')) {
            e.preventDefault();
        } else {
            aj_send({system:"reboot"});
        }
    }, false);
    // shut down Linux
    document.getElementById('sdown').addEventListener('click', function(e) {
        if (!window.confirm('Are you shure to SHUT DOWN?')) {
            e.preventDefault();
        } else {
            aj_send({system:"poweroff"});
        }
    }, false);

}, false);

//
// when connected establishment for WebSocket
//
ws.addEventListener('open', function() {
    // request for MZT data list and initial configration
    aj_send({dblist:"obj", config:"all"});
    document.getElementById('getl').value = "";
}, false);

//
// send by WebSocket
//
function ws_send(data) {
    jsdata = JSON.stringify(data);
    ws.send(jsdata);
}

//
// communication by Ajax
//
function aj_send(data) {
    var xhr = new XMLHttpRequest();
    // Ajaxレスポンス
    xhr.addEventListener('load', function(e) {
        //console.log("received(ajax):" + xhr.responseText);
        var jsdata = JSON.parse(xhr.responseText);
        for ( var key in jsdata ) {
            switch(key) {
                case "tape":    // status for casset tape
                    tape_set(jsdata.tape);
                    break;
                case "titles":  // set MZT list to select-box
                    var sel = document.getElementById('imgname');
                    // delete all items
                    while(sel.lastChild) {
                        sel.removeChild(sel.lastChild);
                    }
                    // add items
                    for ( let tkey in jsdata.titles ) {
                        var opt = document.createElement('option');
                        opt.innerHTML = tkey;
                        opt.setAttribute('value', jsdata.titles[tkey]);
                        sel.appendChild(opt);
                    }
                    aj_send({mztinfo:sel.value});
                    break;
                case "mztinfo": // display information for selected MZT data
                    document.getElementById('d_author').innerHTML = jsdata.mztinfo[4];
                    document.getElementById('d_source').innerHTML = jsdata.mztinfo[5];
                    document.getElementById('d_desc').innerHTML = jsdata.mztinfo[6];
                    break;
                case "cfginfo": // display information for configration
                    document.getElementById('mode').innerHTML = jsdata.cfginfo[0];
                    document.getElementById('mon-rom').innerHTML = jsdata.cfginfo[1];
                    document.getElementById('cg-rom').innerHTML = jsdata.cfginfo[2];
                    tape_set(jsdata.cfginfo[3]);
                    break;
                default:
                    break;
            }
        }
    }, false);
    // request by Ajax
    xhr.open('POST', 'index.html', true);
    xhr.setRequestHeader('content-type', 'application/x-www-form-urlencoded;charset=UTF-8');
    xhr.send(JSON.stringify(data));
}

//
// change visible/invisible progress-bar by state of casset tape insettion
//
function tape_set(mode) {
    var progress = document.querySelector('.frame');
    if ( mode == "set" ) {
        progress.style.opacity = '1.0';
    } else if ( mode == "none" ) {
        progress.style.opacity = '0.0';
    }
}