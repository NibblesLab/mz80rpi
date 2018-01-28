from tornado import ioloop, iostream, web, gen
import tornado.websocket
from tornado.queues import Queue
import os
import sys
import json
import time
import sqlite3
import base64
import tempfile

q = Queue(maxsize=2)
multiple_mode = False
count_success = 0
count_fail = 0

def recv_event(fd, events):
    #print("recv event")
    #print("event=", events)
    if events & io_loop.READ != 0:
        stream_i.read_until(b"\0", recv_fifo)
    elif events & io_loop.ERROR != 0:
        pass

def recv_fifo(data):
    mdata = data[:-1].decode('utf-8')
    #print("receive(emu):"+mdata)
    sdata = {}
    for sts in mdata.split(','):
        kw = sts[:2]
        if kw == "CP":
            try:
                vl = int(sts[2:])
            except:
                vl = 0
            sdata.update({"progress":vl})
        elif kw == "CM":
            try:
                vl = int(sts[2:])
                if vl == 1:
                    sdata.update({"motor":"on"})
                elif vl == 0:
                    sdata.update({"motor":"off"})
            except:
                pass
        elif kw == "LM":
            try:
                vl = int(sts[2:])
                if vl == 2:
                    sdata.update({"led":"green"})
                elif vl == 1:
                    sdata.update({"led":"red"})
                elif vl == 0:
                    sdata.update({"led":"none"})
            except:
                pass
        elif kw == "BU":
            dbfile = sqlite3.connect('mz80rpi.db')
            cur = dbfile.cursor()
            cur.execute('SELECT mrom,cgrom,color,title FROM SETTINGS WHERE startup="YES"')
            row = cur.fetchone()
            cur.execute('SELECT file,title FROM PROGRAMS WHERE rowid=?', (row[0],))
            row_mrom = cur.fetchone()
            send_fifo(b"SMdata/"+row_mrom[0].encode())
            mz80_st.monname = row_mrom[1]
            if row[2] == "BW":
                send_fifo(b"SC0")
            else:
                send_fifo(b"SC1")
            cur.execute('SELECT file,title FROM PROGRAMS WHERE rowid=?', (row[1],))
            row_cgrom = cur.fetchone()
            send_fifo(b"SFdata/"+row_cgrom[0].encode())
            mz80_st.cgname = row_cgrom[1]
            dbfile.close()
            mz80_st.cfgname = row[3]
            send_fifo(b"SR")
        elif kw == "GM":
            try:
                vl = int(sts[2:])
                if vl == 1:
                    sdata.update({"pcg":"ON"})
                elif vl == 0:
                    sdata.update({"pcg":"OFF"})
            except:
                pass
    q.put(json.dumps(sdata))

def send_fifo(data):
    so = os.open('/tmp/cmdxfer', os.O_WRONLY)
    os.write(so, data)
    os.write(so, b"\0")
    os.close(so)

class WebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        #print("open WebSocket")
        self.ioloop = tornado.ioloop.IOLoop.instance()
        self.send()

    def on_message(self, message):
        #print("receive(ws):"+message)
        msdata = json.loads(message)
        for key, value in msdata.items():
            if key == "getl":
                send_fifo(b"K"+value.encode())

    def on_close(self):
        #print("close WebSocket")
        pass

    def send(self):
        self.ioloop.add_timeout(time.time() + 0.1, self.send)
        try:
            mes = q.get_nowait()
            self.write_message(mes)
        except:
            pass

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render('index.html')

    def post(self):
        message = self.request.body.decode('UTF-8')
        #print("receive(ajax):"+message)
        msdata = json.loads(message)
        sdata = {}
        for key, value in msdata.items():
            if key == "getl":
                send_fifo(b"K"+value.encode())
                sdata.update({"msg":"OK"})
            elif key == "pcg":
                if value == "off":
                    send_fifo(b"P0")
                elif value == "on":
                    send_fifo(b"P1")
                sdata.update({"msg":"OK"})
            elif key == "cmt":
                if value == "play":
                    send_fifo(b"CP")
                    sdata.update({"msg":"OK"})
                elif value == "stop":
                    send_fifo(b"CS")
                    sdata.update({"msg":"OK"})
                elif value == "eject":
                    send_fifo(b"CE")
                    mz80_st.tape = "none"
                    sdata.update({"tape":"none"})
                else:
                    send_fifo(b"CTdata/"+mz80_db.getFileName(value).encode())
                    mz80_st.tape = "set"
                    sdata.update({"tape":"set"})
            elif key == "reset":
                send_fifo(b"R")
                sdata.update({"msg":"OK"})
            elif key == "dblist":
                if value == "all":
                    sdata.update({"titles_all":mz80_db.getProgTitles("all")})
                else:
                    sdata.update({"titles":mz80_db.getProgTitles(value)})
            elif key == "mztinfo":
                sdata.update({"mztinfo":mz80_db.getProgData(value)})
            elif key == "config":
                send_fifo(b"SE")
                sdata.update({"cfginfo":mz80_st.getStatus()})
            elif key == "system":
                os.system(value)
                sdata.update({"msg":"OK"})
        self.write(json.dumps(sdata))

class SubHandler_db(tornado.web.RequestHandler):
    def get(self):
        self.render('db.html')

    def post(self):
        message = self.request.body.decode('UTF-8')
        #print("receive(ajax):"+message)
        msdata = json.loads(message)
        sdata = {}
        for key, value in msdata.items():
            if key == "upload":
                if len(value) == 1:
                    mz80_db.setBinaryData(value[0])
                    adata = {}
                    if mz80_db.type == "mzt":
                        adata.update({"mztname":mz80_db.name})
                    else:
                        adata.update({"mztname":value[0][0]})
                    adata.update({"mztattr":mz80_db.attribute})
                    adata.update({"mzttype":mz80_db.type})
                    sdata.update(adata)
                else:
                    count_success = 0
                    count_fail = 0
                    for datas in value:
                        mz80_db.setBinaryData(datas)
                        adata = []
                        if mz80_db.type == "mzt":
                            adata.append(mz80_db.name)
                        else:
                            adata.append(datas[0])
                        adata.append("")
                        adata.append(mz80_db.type)
                        adata.append(mz80_db.attribute)
                        adata.append("")
                        adata.append("")
                        adata.append("")
                        result = mz80_db.registData(adata)
                        if result["dbmsg"][0] == "OK":
                            count_success += 1
                        else:
                            count_fail += 1
                        sdata.update({"msg":"OK"})
                    msg = "Store complete. "
                    if count_success > 0:
                        msg += str(count_success)
                        msg += " succeeded"
                    if count_success > 0 and count_fail > 0:
                        msg += ","
                    if count_fail > 0:
                        msg += str(count_fail)
                        msg += " failed"
                    msg += "."
                    sdata.update({"dbmsg":["OK",msg]})
            elif key == "dblist":
                if value == "all":
                    sdata.update({"titles_all":mz80_db.getProgTitles("all")})
                else:
                    sdata.update({"titles":mz80_db.getProgTitles(value)})
            elif key == "edit":
                sdata.update({"editdata":mz80_db.getProgData(value)})
            elif key == "regdata":
                sdata.update(mz80_db.registData(value))
            elif key == "deldata":
                sdata.update(mz80_db.deleteData(value))
        self.write(json.dumps(sdata))

class SubHandler_config(tornado.web.RequestHandler):
    def get(self):
        self.render('config.html')

    def post(self):
        message = self.request.body.decode('UTF-8')
        #print("receive(ajax):"+message)
        msdata = json.loads(message)
        sdata = {}
        for key, value in msdata.items():
            if key == "dblist":
                if value == "all":
                    sdata.update({"titles_all":mz80_db.getProgTitles("all")})
                else:
                    sdata.update({"titles":mz80_db.getProgTitles(value)})
            elif key == "regconf":
                sdata.update(mz80_db.registConf(value, False))
            elif key == "updconf":
                sdata.update(mz80_db.registConf(value, True))
            elif key == "cflist":
                sdata.update({"titles_cf":mz80_db.getConfTitles(value)})
            elif key == "cfedit":
                sdata.update({"editdata":mz80_db.getConfData(value)})
            elif key == "delconf":
                sdata.update(mz80_db.deleteConf(value))
            elif key == "setconf":
                send_fifo(b"SS")
                dbfile = sqlite3.connect('mz80rpi.db')
                cur = dbfile.cursor()
                cur.execute('SELECT file,title FROM PROGRAMS WHERE rowid=?', (value[1],))
                row_mrom = cur.fetchone()
                send_fifo(b"SMdata/"+row_mrom[0].encode())
                mz80_st.monname = row_mrom[1]
                if value[3] == "BW":
                    send_fifo(b"SC0")
                else:
                    send_fifo(b"SC1")
                cur.execute('SELECT file,title FROM PROGRAMS WHERE rowid=?', (value[2],))
                row_cgrom = cur.fetchone()
                send_fifo(b"SFdata/"+row_cgrom[0].encode())
                mz80_st.cgname = row_cgrom[1]
                dbfile.close()
                mz80_st.cfgname = value[0]
                send_fifo(b"R")
                send_fifo(b"SR")
                sdata.update({"msg":"OK"})
        self.write(json.dumps(sdata))

class Application(tornado.web.Application):
    def __init__(self):
        handlers = [
            (r"/", MainHandler),
            (r"/index.html", MainHandler),
            (r"/db.html", SubHandler_db),
            (r"/config.html", SubHandler_config),
            (r"/websocket", WebSocketHandler),
        ]
        settings = {
            "template_path": ".",
            "static_path": ".",
            "websocket_ping_interval": 1
        }
        tornado.web.Application.__init__(self, handlers, **settings)

class dbAccess():
    def __init__(self):
        self.attrtypes = [1, 2, 3, 4, 5, 7, 10]
        self.cnv_list_j = ['日', '月', '火', '水', '木', '金', '土', '生', '年', '時', '分', '秒', '円', '￥', '£',  '_',
                           '_',  '。', '「', '」', '、', '．', 'ヲ', 'ァ', 'ィ', 'ゥ', 'ェ', 'ォ', 'ャ', 'ュ', 'ョ', 'ッ',
                           'ー', 'ア', 'イ', 'ウ', 'エ', 'オ', 'カ', 'キ', 'ク', 'ケ', 'コ', 'サ', 'シ', 'ス', 'セ', 'ソ',
                           'タ', 'チ', 'ツ', 'テ', 'ト', 'ナ', 'ニ', 'ヌ', 'ネ', 'ノ', 'ハ', 'ヒ', 'フ', 'ヘ', 'ホ', 'マ',
                           'ミ', 'ム', 'メ', 'モ', 'ヤ', 'ユ', 'ヨ', 'ラ', 'リ', 'ル', 'レ', 'ロ', 'ワ', 'ン', '゛', '゜']
        self.cnv_list_e = ['_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',
                           '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',
                           '_', '_', 'e', '_', '_', '_', 't', 'g', 'h', '_', 'b', 'x', 'd', 'r', 'p', 'c',
                           'q', 'a', 'z', 'w', 's', 'u', 'i', '_', 'Ö', 'k', 'f', 'v', '_', 'ü', 'ß', 'j',
                           'n', '_', 'Ü', 'm', '_', '_', '_', 'o', 'l', 'Ä', 'ö', 'ä', '_', 'y', '_', '_']

    def setBinaryData(self, data):
        dat = data[1].split(',')
        self.bindata = base64.standard_b64decode(dat[1])
        self.updateDataFlag = False
        if data[0][-3:].upper() == 'MZF':
            self.code_uk = True
        else:
            self.code_uk = False
        self.name = self.getMztName()
        self.type = self.isKindOfMztData()
        self.attribute = self.isAttrOfMztData()

    def getMztName(self):
        fnam = ''
        for b in self.bindata[1:16]:
            if b == 13:
                break
            elif b < 32:
                fnam += '_'
            elif b > 31 and b < 92:
                fnam += chr(b)
            elif b == 92:
                fnam += '\\'
            elif b == 93:
                fnam += ']'
            elif b > 93 and b < 112:
                fnam += '_'
            elif b > 111 and b < 192:
                if self.code_uk:
                    fnam += self.cnv_list_e[b - 112]
                else:
                    fnam += self.cnv_list_j[b - 112]
            elif b == 251 and self.code_uk:
                fnam += '£'
            elif b > 191:
                fnam += '_'
        return fnam

    def isKindOfMztData(self):
        if (self.bindata[0] in self.attrtypes) == False:
            return 'rom'
        else:
            return 'mzt'

    def isAttrOfMztData(self):
        if self.bindata[0] == 1:
            return 'obj'
        elif self.bindata[0] == 2:
            return 'btx'
        elif self.bindata[0] == 3:
            return 'bsd'
        else:
            return 'oth'

    def getProgTitles(self, type):
        dbfile = sqlite3.connect('mz80rpi.db')
        dbfile.row_factory = sqlite3.Row
        cur = dbfile.cursor()
        sdata = {}
        if type == 'all':
            for row in cur.execute('SELECT title,rowid FROM PROGRAMS ORDER BY title ASC'):
                sdata.update({row['title']:row['rowid']})
        elif type == 'rom':
            for row in cur.execute('SELECT title,rowid FROM PROGRAMS WHERE fkind="ROM" ORDER BY title ASC'):
                sdata.update({row['title']:row['rowid']})
        else:
            for row in cur.execute('SELECT title,rowid FROM PROGRAMS WHERE fkind="MZT" and attribute=? ORDER BY title ASC', (type.upper(),)):
                sdata.update({row['title']:row['rowid']})
        dbfile.close()
        return sdata

    def getProgData(self, rowid):
        dbfile = sqlite3.connect('mz80rpi.db')
        cur = dbfile.cursor()
        cur.execute('SELECT * FROM PROGRAMS WHERE rowid=?', (rowid,))
        row = cur.fetchone()
        dbfile.close()
        self.updateDataFlag = True
        self.updateRowid = rowid
        return row

    def getFileName(self, rowid):
        dbfile = sqlite3.connect('mz80rpi.db')
        cur = dbfile.cursor()
        cur.execute('SELECT file FROM PROGRAMS WHERE rowid=?', (rowid,))
        row = cur.fetchone()
        dbfile.close()
        return row[0]

    def registData(self, data):
        dbfile = sqlite3.connect('mz80rpi.db')
        cur = dbfile.cursor()
        if self.updateDataFlag:
            try:
                cur.execute('UPDATE PROGRAMS SET title=?, fkind=?, attribute=?, author=?, source=?, description=? WHERE rowid=?', (data[0],data[2].upper(),data[3].upper(),data[4],data[5],data[6],self.updateRowid))
                rdata = {"dbmsg":["OK","Update successfully:"+data[0]]}
            except sqlite3.Error as e:
                print("SQlite3 error(UPDATE):", e.args[0])
                rdata = {"dbmsg":["NG","Update error:"+data[0]]}
        else:
            fp, path = tempfile.mkstemp(dir='../emu/data', prefix='mzD_')
            os.write(fp, self.bindata)
            os.close(fp)
            try:
                cur.execute('INSERT INTO PROGRAMS VALUES(?,?,?,?,?,?,?)', (data[0],path.split('/')[-1:][0],data[2].upper(),data[3].upper(),data[4],data[5],data[6]))
                rdata = {"dbmsg":["OK","Store successfully:"+data[0]]}
            except sqlite3.Error as e:
                print("SQlite3 error(INSERT):", e.args[0])
                rdata = {"dbmsg":["NG","Store error:"+data[0]]}
        dbfile.commit()
        dbfile.close()
        return rdata

    def deleteData(self, rowid):
        dbfile = sqlite3.connect('mz80rpi.db')
        cur = dbfile.cursor()
        cur.execute('SELECT file,title FROM PROGRAMS WHERE rowid=?', (rowid,))
        row = cur.fetchone()
        os.remove(b"../emu/data/"+row[0].encode())
        try:
            cur.execute('DELETE FROM PROGRAMS WHERE rowid=?', (rowid,))
            rdata = {"dbmsg":["OK","Delete successfully:"+row[1]]}
        except sqlite3.Error as e:
            print("SQlite3 error(DELETE):", e.args[0])
            rdata = {"dbmsg":["NG","Delete error:"+row[1]]}
        dbfile.commit()
        dbfile.close()
        return rdata

    def registConf(self, data, updateFlag):
        dbfile = sqlite3.connect('mz80rpi.db')
        cur = dbfile.cursor()
        if data[4] == "YES":
            try:
                cur.execute('UPDATE SETTINGS SET startup="NO" WHERE startup="YES"')
            except sqlite3.Error as e:
                print("SQlite3 error(UPDATE):", e.args[0])
        if updateFlag:
            try:
                cur.execute('UPDATE SETTINGS SET title=?, mrom=?, cgrom=?, color=?, startup=? WHERE rowid=?', (data[0],data[1],data[2],data[3],data[4],self.updateRowid))
                rdata = {"dbmsg":["OK","Update successfully:"+data[0]]}
            except sqlite3.Error as e:
                print("SQlite3 error(UPDATE):", e.args[0])
                rdata = {"dbmsg":["NG","Update error:"+data[0]]}
        else:
            try:
                cur.execute('INSERT INTO SETTINGS VALUES(?,?,?,?,?)', (data[0],data[1],data[2],data[3],data[4]))
                rdata = {"dbmsg":["OK","Store successfully:"+data[0]]}
            except sqlite3.Error as e:
                print("SQlite3 error(INSERT):", e.args[0])
                rdata = {"dbmsg":["NG","Store error:"+data[0]]}
        dbfile.commit()
        dbfile.close()
        return rdata

    def getConfTitles(self, type):
        dbfile = sqlite3.connect('mz80rpi.db')
        dbfile.row_factory = sqlite3.Row
        cur = dbfile.cursor()
        sdata = {}
        for row in cur.execute('SELECT title,rowid FROM SETTINGS ORDER BY title ASC'):
            sdata.update({row['title']:row['rowid']})
        dbfile.close()
        return sdata

    def getConfData(self, rowid):
        dbfile = sqlite3.connect('mz80rpi.db')
        cur = dbfile.cursor()
        cur.execute('SELECT * FROM SETTINGS WHERE rowid=?', (rowid,))
        row = cur.fetchone()
        dbfile.close()
        self.updateRowid = rowid
        return row

    def deleteConf(self, rowid):
        dbfile = sqlite3.connect('mz80rpi.db')
        cur = dbfile.cursor()
        cur.execute('SELECT title FROM SETTINGS WHERE rowid=?', (rowid,))
        row = cur.fetchone()
        try:
            cur.execute('DELETE FROM SETTINGS WHERE rowid=?', (rowid,))
            rdata = {"dbmsg":["OK","Delete successfully:"+row[0]]}
        except sqlite3.Error as e:
            print("SQlite3 error(DELETE):", e.args[0])
            rdata = {"dbmsg":["NG","Delete error:"+row[0]]}
        dbfile.commit()
        dbfile.close()
        return rdata

    def getVersion(self):
        dbfile = sqlite3.connect('mz80rpi.db')
        cur = dbfile.cursor()
        cur.execute('SELECT ver FROM VERSION')
        row = cur.fetchone()
        dbfile.close()
        return row[0]

class Emu_Status():
    def __init__(self):
        self.cfgname = ""
        self.monname = ""
        self.cgname = ""
        self.tape = "none"

    def getStatus(self):
        return [self.cfgname, self.monname, self.cgname, self.tape]

if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    si = os.open('/tmp/stsxfer', os.O_RDONLY|os.O_NONBLOCK)
    stream_i = tornado.iostream.PipeIOStream(si)
    io_loop = tornado.ioloop.IOLoop.current()
    io_loop.add_handler(si, recv_event, io_loop.READ)
    http_server = tornado.httpserver.HTTPServer(Application())
    http_server.listen(8080)
    mz80_db = dbAccess()
    if mz80_db.getVersion() != 1.0:
        print("DB version is not matched. DB:%s require:1.0" % mz80_db.getVersion())
        exit()
    mz80_st = Emu_Status()
    io_loop.start()
