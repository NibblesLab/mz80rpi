var ws;
ws = new WebSocket("ws://192.168.1.199:8080/websocket");

//var progress = document.querySelector('.percent');

document.addEventListener('DOMContentLoaded', function() {
    // WebSocket 受信
    ws.addEventListener('message', function (event) {
        console.log("received:" + event.data);
        jsdata = JSON.parse(event.data);
        for (var key in jsdata) {
            switch(key) {
                case "progress":
                    var progress = document.querySelector('.percent');
                    progress.style.width = jsdata.progress + '%';
                    break;
                case "tape":
                    var progress = document.querySelector('#progress_bar');
                    if (jsdata.tape == "set") {
                        progress.style.opacity = '1.0';
                        //document.getElementById('progress_bar').className = 'tapeset';
                    } else if (jsdata.tape == "none") {
                        progress.style.opacity = '0.0';
                        //document.getElementById('progress_bar').className = '';
                    }
                //    var text = getElementById('casset');
                //    text.textContent = jsdata[key];
                    break;
                case "motor":
                    var progress = document.querySelector('.percent');
                    if (jsdata.motor == "on") {
                        progress.style.backgroundColor = '#99ccff';
                    } else if (jsdata.motor == "off") {
                        progress.style.backgroundColor = '#808080';
                    }
                    break;
                default:
                    break;
            }
        }
    }, false);

    // リセット
    document.getElementById('zrst').addEventListener('click', function(e) {
        if (!window.confirm('実行しますか？')) {
            e.preventDefault();
        } else {
            ws_send({reset:"cpu"});
        }
    }, false);
    // PCG ON
    document.getElementById('pcg0').addEventListener('click', function(e) {
        ws_send({pcg:"off"});
        document.getElementById('pcg0').disabled = true;
        document.getElementById('pcg1').disabled = false;
    }, false);
    // PCG OFF
    document.getElementById('pcg1').addEventListener('click', function(e) {
        ws_send({pcg:"on"});
        document.getElementById('pcg0').disabled = false;
        document.getElementById('pcg1').disabled = true;
    }, false);
    // CMT 再生
    document.getElementById('play').addEventListener('click', function(e) {
        ws_send({cmt:"play"});
    }, false);
    // CMT 停止
    document.getElementById('stop').addEventListener('click', function(e) {
        ws_send({cmt:"stop"});
    }, false);
    // CMT 取り出し
    document.getElementById('eject').addEventListener('click', function(e) {
        ws_send({cmt:"eject"});
    }, false);
    // MZT data set
    document.getElementById('setcmt').addEventListener('click', function(e) {
        ws_send({cmt:document.getElementById('cmtpath').value});
    }, false);
    // Input 1 line
    document.getElementById('cr').addEventListener('click', function(e) {
        ws_send({getl:document.getElementById('getl').value});
    }, false);
    document.getElementById('getl').addEventListener('keypress', function(e) {
        if (e.keyCode == 13) {
            ws_send({getl:document.getElementById('getl').value});
        }
    }, false);
}, false);

function ws_send(data) {
    jsdata = JSON.stringify(data);
    ws.send(jsdata);
}