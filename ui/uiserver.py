from tornado import ioloop, iostream, web, gen
import tornado.websocket
from tornado.queues import Queue
import os
import sys
import json
import time

q = Queue(maxsize=2)

def recv_event(fd, events):
    #print("recv event")
    #print("event=", events)
    if events & io_loop.READ != 0:
        stream_i.read_until(b"\0", recv_fifo)
    elif events & io_loop.ERROR != 0:
        pass

def recv_fifo(data):
    #print("recv data")
    mdata = data[:-1].decode('utf-8')
    print("receive(emu):"+mdata)
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
    #print(json.dumps(sdata))
    #tornado.websocket.WebSocketHandler.write_message(json.dumps(sdata))
    q.put(json.dumps(sdata))

def send_fifo(data):
    so = os.open('/tmp/cmdxfer', os.O_WRONLY)
    os.write(so, data)
    os.write(so, b"\0")
    os.close(so)

class WebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        print("open WebSocket")
        self.ioloop = tornado.ioloop.IOLoop.instance()
        self.send()

    def on_message(self, message):
        print("receive(web):"+message)
        msdata = json.loads(message)
        for key, value in msdata.items():
            if key == "getl":
                send_fifo(b"K"+value.encode())
            elif key == "pcg":
                if value == "off":
                    send_fifo(b"P0")
                elif value == "on":
                    send_fifo(b"P1")
            elif key == "cmt":
                if value == "play":
                    send_fifo(b"CP")
                elif value == "stop":
                    send_fifo(b"CS")
                elif value == "eject":
                    send_fifo(b"CE")
                    q.put(json.dumps({"tape":"none"}))
                else:
                    send_fifo(b"CT"+value.encode())
                    q.put(json.dumps({"tape":"set"}))
            elif key == "reset":
                send_fifo(b"R")

    def on_close(self):
        print("close WebSocket")

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

class Application(tornado.web.Application):
    def __init__(self):
        handlers = [
            (r"/", MainHandler),
            (r"/websocket", WebSocketHandler),
        ]
        settings = {
            "static_path": os.path.join(os.path.dirname(__file__), "."),
            "websocket_ping_interval": 1
        }
        tornado.web.Application.__init__(self, handlers, **settings)

if __name__ == "__main__":
    si = os.open('/tmp/stsxfer', os.O_RDONLY|os.O_NONBLOCK)
    stream_i = tornado.iostream.PipeIOStream(si)
    io_loop = tornado.ioloop.IOLoop.current()
    io_loop.add_handler(si, recv_event, io_loop.READ)
    http_server = tornado.httpserver.HTTPServer(Application())
    http_server.listen(8080)
    io_loop.start()
