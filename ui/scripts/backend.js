var ws;
ws = new WebSocket("ws://192.168.1.199:8080/websocket");

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('zrst').addEventListener('click', function(e) {
        if (!window.confirm('実行しますか？')) {
            e.preventDefault();
        } else {
            ws.send("R");
        }
    }, false);
}, false);

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('pcg0').addEventListener('click', function(e) {
        ws.send("P0");
        document.getElementById('pcg0').disabled = true;
        document.getElementById('pcg1').disabled = false;
    }, false);
}, false);

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('pcg1').addEventListener('click', function(e) {
        ws.send("P1");
        document.getElementById('pcg0').disabled = false;
        document.getElementById('pcg1').disabled = true;
    }, false);
}, false);

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('play').addEventListener('click', function(e) {
        ws.send("CP");
    }, false);
}, false);

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('stop').addEventListener('click', function(e) {
        ws.send("CS");
    }, false);
}, false);

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('eject').addEventListener('click', function(e) {
        ws.send("CE");
    }, false);
}, false);

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('setcmt').addEventListener('click', function(e) {
        ws.send("CT"+document.getElementById('cmtpath').value);
    }, false);
}, false);

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('cr').addEventListener('click', function(e) {
        ws.send("K"+document.getElementById('key1line').value);
    }, false);
}, false);
