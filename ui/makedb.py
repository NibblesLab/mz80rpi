import sqlite3

dbfile = sqlite3.connect('mz80rpi.db')
cur = dbfile.cursor()
cur.execute("CREATE TABLE PROGRAMS (title, file, fkind, attribute, author, source, description)")
cur.execute("CREATE TABLE SETTINGS (title, mrom, cgrom, color, startup)")
cur.execute("CREATE TABLE VERSION (ver)")
cur.execute("INSERT INTO VERSION VALUES(1.0)")
dbfile.commit()
dbfile.close()
