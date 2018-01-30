var recallData = [];

document.addEventListener('DOMContentLoaded', function() {
    // Re-load
    document.getElementById('reload').addEventListener('click', function(e) {
        location.reload(true);
    }, false);
    // Control buttons
    document.getElementById('selset').addEventListener('click', function(e) {
        aj_send({cfedit:document.getElementById('setname').value});
    }, false);
    document.getElementById('s_delete').addEventListener('click', function(e) {
        if (!window.confirm('Are you shure to delete setting?')) {
            e.preventDefault();
        } else {
            aj_send({delconf:document.getElementById('setname').value});
        }
    }, false);
    document.getElementById('s_cancel').addEventListener('click', function(e) {
        clearForm();
    }, false);
    document.getElementById('s_apply').addEventListener('click', function(e) {
        var rowdata = [];
        rowdata.push(document.getElementById('c_name').value);
        rowdata.push(document.getElementById('monimg').value);
        rowdata.push(document.getElementById('cgimg').value);
        if ( document.getElementById("bw").checked ) {
            rowdata.push("BW");
        } else if ( document.getElementById("green").checked ) {
            rowdata.push("GREEN");
        }
        if ( rowdata.length == 4 ){
            if ( rowdata[0] != recallData[0] || rowdata[1] != recallData[1] || rowdata[2] != recallData[2] || rowdata[3] != recallData[3]) {
                rowdata[0] = "(modified)";
            }
            aj_send({setconf:rowdata});
        }
    }, false);
    document.getElementById('s_new').addEventListener('click', function(e) {
        var rowdata = [];
        rowdata.push(document.getElementById('c_name').value);
        rowdata.push(document.getElementById('monimg').value);
        rowdata.push(document.getElementById('cgimg').value);
        if ( document.getElementById("bw").checked ) {
            rowdata.push("BW");
        } else if ( document.getElementById("green").checked ) {
            rowdata.push("GREEN");
        }
        if ( document.getElementById("s_yes").checked ) {
            rowdata.push("YES");
        } else if ( document.getElementById("s_no").checked ) {
            rowdata.push("NO");
        }
        if ( rowdata.length == 5 ){
            aj_send({regconf:rowdata});
        }
    }, false);
    document.getElementById('s_update').addEventListener('click', function(e) {
        var rowdata = [];
        rowdata.push(document.getElementById('c_name').value);
        rowdata.push(document.getElementById('monimg').value);
        rowdata.push(document.getElementById('cgimg').value);
        if ( document.getElementById("bw").checked ) {
            rowdata.push("BW");
        } else if ( document.getElementById("green").checked ) {
            rowdata.push("GREEN");
        }
        if ( document.getElementById("s_yes").checked ) {
            rowdata.push("YES");
        } else if ( document.getElementById("s_no").checked ) {
            rowdata.push("NO");
        }
        if ( rowdata.length == 5 ){
            aj_send({updconf:rowdata});
        }
    }, false);
    document.getElementById('monimg').addEventListener('change', function(e) {
        if( document.getElementById('s_update').disabled == false ) {
            document.getElementById('s_new').disabled = false;
            document.getElementById('s_delete').disabled = true;
        } else {
            if ( document.getElementById('s_cancel').disabled == false ) {
                document.getElementById('s_new').disabled = false;
            }
        }
    });
    document.getElementById('cgimg').addEventListener('change', function(e) {
        if( document.getElementById('s_update').disabled == false ) {
            document.getElementById('s_new').disabled = false;
            document.getElementById('s_delete').disabled = true;
        } else {
            if ( document.getElementById('s_cancel').disabled == false ) {
                document.getElementById('s_new').disabled = false;
            }
        }
    });
    document.getElementById('c_name').addEventListener('keyup', function(e) {
        if ( document.getElementById('c_name').value == "" ) {
            document.getElementById('s_new').disabled = true;
        } else {
            if ( (document.getElementById('bw').checked | document.getElementById('green').checked) & (document.getElementById('s_yes').checked | document.getElementById('s_no').checked) ) {
                document.getElementById('s_cancel').disabled = false;
                document.getElementById('s_apply').disabled = false;
                document.getElementById('s_new').disabled = false;
            }
        }
    }, false);
    document.getElementById('bw').addEventListener('change', function(e) {
        document.getElementById('s_apply').disabled = false;
        if ( (document.getElementById('s_yes').checked | document.getElementById('s_no').checked) ) {
            if ( document.getElementById('c_name').value != "" ) {
                document.getElementById('s_new').disabled = false;
            }
        }
        document.getElementById('s_cancel').disabled = false;
    }, false);
    document.getElementById('green').addEventListener('change', function(e) {
        document.getElementById('s_apply').disabled = false;
        if ( (document.getElementById('s_yes').checked | document.getElementById('s_no').checked) ) {
            if ( document.getElementById('c_name').value != "" ) {
                document.getElementById('s_new').disabled = false;
            }
        }
        document.getElementById('s_cancel').disabled = false;
    }, false);
    document.getElementById('s_yes').addEventListener('change', function(e) {
        if ( (document.getElementById('bw').checked | document.getElementById('green').checked) ) {
            if ( document.getElementById('c_name').value != "" ) {
                document.getElementById('s_new').disabled = false;
            }
        }
        document.getElementById('s_cancel').disabled = false;
    }, false);
    document.getElementById('s_no').addEventListener('change', function(e) {
        if ( (document.getElementById('bw').checked | document.getElementById('green').checked) ) {
            if ( document.getElementById('c_name').value != "" ) {
                document.getElementById('s_new').disabled = false;
            }
        }
        document.getElementById('s_cancel').disabled = false;
    }, false);

}, false);

window.addEventListener('load', function() {
    clearForm();
    // コンフィグデータリスト要求
    aj_send({cflist:"all", dblist:"rom"});
}, false);

function clearForm() {
    document.getElementById('s_delete').disabled = true;
    document.getElementById('s_cancel').disabled = true;
    document.getElementById('s_apply').disabled = true;
    document.getElementById('s_new').disabled = true;
    document.getElementById('s_update').disabled = true;
    document.getElementById('c_name').value = "";
    document.getElementById('bw').checked = false;
    document.getElementById('green').checked = false;
    document.getElementById('s_yes').checked = false;
    document.getElementById('s_no').checked = false;
    recallData = [];
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
                case "titles":
                    var monsel = document.getElementById('monimg');
                    var cgsel = document.getElementById('cgimg');
                    // 一旦削除
                    while(monsel.lastChild) {
                        monsel.removeChild(monsel.lastChild);
                    }
                    while(cgsel.lastChild) {
                        cgsel.removeChild(cgsel.lastChild);
                    }
                    // アイテムを追加
                    for ( let tkey in jsdata.titles ) {
                        var optm = document.createElement('option');
                        var optc = document.createElement('option');
                        optm.innerHTML = tkey;
                        optc.innerHTML = tkey;
                        optm.setAttribute('value', jsdata.titles[tkey]);
                        optc.setAttribute('value', jsdata.titles[tkey]);
                        monsel.appendChild(optm);
                        cgsel.appendChild(optc);
                    }
                    break;
                case "titles_cf":
                    var sel = document.getElementById('setname');
                    // 一旦削除
                    while(sel.lastChild) {
                        sel.removeChild(sel.lastChild);
                    }
                    // アイテムを追加
                    for ( let tkey in jsdata.titles_cf ) {
                        var opt = document.createElement('option');
                        opt.innerHTML = tkey;
                        opt.setAttribute('value', jsdata.titles_cf[tkey]);
                        sel.appendChild(opt);
                    }
                    break;
                case "editdata":
                    document.getElementById('c_name').value = jsdata.editdata[0];
                    recallData.push(jsdata.editdata[0]);
                    document.getElementById('monimg').value = jsdata.editdata[1];
                    recallData.push(jsdata.editdata[1]);
                    document.getElementById('cgimg').value = jsdata.editdata[2];
                    recallData.push(jsdata.editdata[2]);
                    if ( jsdata.editdata[3] == "BW" ) {
                        document.getElementById('bw').checked = true;
                    } else {
                        document.getElementById('green').checked = true;
                    }
                    recallData.push(jsdata.editdata[3]);
                    if ( jsdata.editdata[4] == "YES" ) {
                        document.getElementById('s_yes').checked = true;
                    } else {
                        document.getElementById('s_no').checked = true;
                    }
                    document.getElementById('s_delete').disabled = false;
                    document.getElementById('s_apply').disabled = false;
                    document.getElementById('s_update').disabled = false;
                    document.getElementById('s_cancel').disabled = false;
                    break;
                case "dbmsg":
                    var msg = document.querySelector('#message');
                    if ( jsdata.dbmsg[0] == "OK") {
                        msg.style.color = '#0000ff';
                        document.getElementById('s_new').disabled = true;
                        document.getElementById('s_update').disabled = true;
                        aj_send({cflist:"all"});
                    } else {
                        msg.style.color = '#ff0000';
                    }
                    document.getElementById('message').innerHTML = jsdata.dbmsg[1];
                    break;
                default:
                    break;
            }
        }
    }, false);
    // request by Ajax
    xhr.open('POST', 'config.html', true);
    xhr.setRequestHeader('content-type', 'application/x-www-form-urlencoded;charset=UTF-8');
    xhr.send(JSON.stringify(data));
}
