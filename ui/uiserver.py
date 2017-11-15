import tornado.ioloop
import tornado.web
import tornado.websocket
import os
import sys
import socket

class UnixDomainClient:
    def __init__(self, socket_path):
        self.socket_path = socket_path

    def start(self, cmd):
        s = self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s.connect(self.socket_path)
        message = cmd + "\0"
        print(cmd)
        #sys.stdout.write("send to server ({}): {}\n".format(idx, message))
        s.send(message.encode())
        s.close()

class WebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        print("open WebSocket")

    def on_message(self, message):
        print("receive:"+message)
        client.start(message)

    def on_close(self):
        print("close WebSocket")

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
            "static_path": os.path.join(os.path.dirname(__file__), ".")
        }
        tornado.web.Application.__init__(self, handlers, **settings)

if __name__ == "__main__":
    client = UnixDomainClient('/tmp/cmdxfer')
    http_server = tornado.httpserver.HTTPServer(Application())
    http_server.listen(8080)
    tornado.ioloop.IOLoop.current().start()
