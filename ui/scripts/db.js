
document.addEventListener('DOMContentLoaded', function() {
    // Re-load
    document.getElementById('reload').addEventListener('click', function(e) {
        location.reload(true);
    }, false);
    // DB登録・編集
    document.getElementById('e_name').addEventListener('keyup', function(e) {
        if ( document.getElementById('e_name').value == "" ) {
            document.getElementById('e_exec').disabled = true;
        } else {
            document.getElementById('e_exec').disabled = false;
        }
    }, false);
    document.getElementById('e_cancel').addEventListener('click', function(e) {
        set_dbfunc();
    }, false);
    document.getElementById('e_exec').addEventListener('click', function(e) {
        var rowdata = [];
        rowdata.push(document.getElementById('e_name').value);
        rowdata.push("");
        if ( document.getElementById("k_rom").checked ) {
            rowdata.push("ROM");
        } else if ( document.getElementById("k_mzt").checked ) {
            rowdata.push("MZT");
        }
        var item = document.getElementsByName('e_fattr');
        for ( var i = 0, len = item.length; i < len; i++) {
            if ( item.item(i).checked ) {
                rowdata.push(item.item(i).value);
            }
        }
        rowdata.push(document.getElementById('e_author').value);
        rowdata.push(document.getElementById('e_source').value);
        rowdata.push(document.getElementById('e_desc').value);
        if ( rowdata.length == 7 ){
            aj_send({regdata:rowdata});
        }
    }, false);
    document.getElementById('e_delete').addEventListener('click', function(e) {
        if (!window.confirm('Are you shure to delete data?')) {
            e.preventDefault();
        } else {
            aj_send({deldata:document.getElementById('e_imgname').value});
        }
    }, false);

}, false);

window.addEventListener('load', function() {
    // DB登録/修正選択ボタンの配置
    set_dbfunc();
}, false);

function setType(type) {
    var item = document.getElementsByName('e_fattr');
    if ( type.toLowerCase() == "rom" ) {
        for ( var i = 0, len = item.length; i < len; i++) {
            item.item(i).disabled = true;
        }
        document.getElementById("k_rom").checked = true;
    } else if ( type.toLowerCase() == "mzt" ) {
        for ( var i = 0, len = item.length; i < len; i++ ) {
            item.item(i).disabled = false;
        }
        document.getElementById("k_mzt").checked = true;
    } else {
        document.getElementById("k_rom").checked = false;
        document.getElementById("k_mzt").checked = false;
    }
}

function setAttr(attr) {
    var item = document.getElementsByName('e_fattr');
    for ( var i = 0, len = item.length; i < len; i++ ) {
        item.item(i).checked = false;
        if ( item.item(i).value == attr.toLowerCase() ) {
            item.item(i).checked = true;
        }
    }
}

function set_dbfunc() {
    var item = document.getElementById('db_selfunc');
    while(item.lastChild) {
        item.removeChild(item.lastChild);
    }
    var label = document.createElement('label');
    label.setAttribute('for', 'newitem');
    label.innerHTML = "Select Function:";
    item.appendChild(label);
    var button1 = document.createElement('button');
    button1.setAttribute('id', 'newitem');
    button1.innerHTML = "New...";
    button1.addEventListener('click', set_newitem, false);
    item.appendChild(button1);
    var button2 = document.createElement('button');
    button2.setAttribute('id', 'edititem');
    button2.innerHTML = "Edit...";
    button2.addEventListener('click', set_edititem, false);
    item.appendChild(button2);
    document.getElementById('e_delete').disabled = true;
    document.getElementById('e_cancel').disabled = true;
    document.getElementById('e_exec').disabled = true;
    document.getElementById('e_name').value = "";
    setType("");
    setAttr("");
    document.getElementById('e_author').value = "";
    document.getElementById('e_source').value = "";
    document.getElementById('e_desc').value = "";
}

function set_newitem() {
    // ボタンイベント削除
    document.getElementById('newitem').removeEventListener('click', set_newitem);
    document.getElementById('edititem').removeEventListener('click', set_edititem);
    // ラベルとボタンを除去
    var item = document.getElementById('db_selfunc');
    while(item.lastChild) {
        item.removeChild(item.lastChild);
    }
    // ラベルとファイル選択ボタンを設置
    var label = document.createElement('label');
    label.setAttribute('for', 'file');
    label.innerHTML = "Select File:";
    item.appendChild(label);
    var fsel = document.createElement('input');
    fsel.setAttribute('id', 'file');
    fsel.setAttribute('type', 'file');
    fsel.multiple = true;
    fsel.addEventListener('change', function(e) {
        var datas = [];
        var pending = 0;
        Array.prototype.forEach.call(fsel.files, function(file, index) {
            var reader = new FileReader();
            reader.onload = function() {
                datas.push([file.name,reader.result]);
                --pending;
                if ( pending == 0 ){
                    aj_send({upload:datas});
                }
            }
            reader.readAsDataURL(file);
            pending++;
        });
    }, true);
    item.appendChild(fsel);
    // キャンセルボタンだけ有効化
    document.getElementById('e_cancel').disabled = false;
}

function set_edititem() {
    // ボタンイベント削除
    document.getElementById('newitem').removeEventListener('click', set_newitem);
    document.getElementById('edititem').removeEventListener('click', set_edititem);
    // ラベルとボタンを除去
    var item = document.getElementById('db_selfunc');
    while(item.lastChild) {
        item.removeChild(item.lastChild);
    }
    // リスト取得
    aj_send({dblist:"all"});
    // キャンセルボタンだけ有効化
    document.getElementById('e_cancel').disabled = false;
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
                case "titles_all":
                    var item = document.getElementById('db_selfunc');
                    var label = document.createElement('label');
                    label.setAttribute('for', 'e_imgname');
                    label.innerHTML = "Select Name:";
                    item.appendChild(label);
                    var sel = document.createElement('select');
                    sel.setAttribute('id', 'e_imgname');
                    sel.setAttribute('name', 'e_imgname');
                    // アイテムを追加
                    for ( let tkey in jsdata.titles_all ) {
                        var opt = document.createElement('option');
                        opt.innerHTML = tkey;
                        opt.setAttribute('value', jsdata.titles_all[tkey]);
                        sel.appendChild(opt);
                    }
                    item.appendChild(sel);
                    var button = document.createElement('button');
                    button.setAttribute('id', 'setitem');
                    button.innerHTML = "SELECT";
                    button.addEventListener('click', function(e) {
                        aj_send({edit:sel.value});
                    }, false);
                    item.appendChild(button);
                    break;
                case "editdata":
                    document.getElementById('e_name').value = jsdata.editdata[0];
                    setType(jsdata.editdata[2]);
                    setAttr(jsdata.editdata[3]);
                    document.getElementById('e_author').value = jsdata.editdata[4];
                    document.getElementById('e_source').value = jsdata.editdata[5];
                    document.getElementById('e_desc').value = jsdata.editdata[6];
                    document.getElementById('e_delete').disabled = false;
                    document.getElementById('e_cancel').disabled = false;
                    document.getElementById('e_exec').disabled = false;
                    break;
                case "mztname":
                    document.getElementById('e_name').value = jsdata.mztname;
                    break;
                case "mzttype":
                    setType(jsdata.mzttype);
                    break;
                case "mztattr":
                    setAttr(jsdata.mztattr);
                    document.getElementById('e_exec').disabled = false;
                    break;
                case "dbmsg":
                    var msg = document.querySelector('#message');
                    if ( jsdata.dbmsg[0] == "OK") {
                        msg.style.color = '#0000ff';
                        set_dbfunc();
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
    xhr.open('POST', 'db.html', true);
    xhr.setRequestHeader('content-type', 'application/x-www-form-urlencoded;charset=UTF-8');
    xhr.send(JSON.stringify(data));
}
