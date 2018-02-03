import sys

print('Font Pattern Generator for mz80rpi')

try:
    infile = open(sys.argv[1], 'rt')
except IndexError as err:
    print('[Usage]python3 fontgen.py textfile')
    exit()
except FileNotFoundError as err:
    print('File not found : %s' % sys.argv[1])
    exit()
romdata = bytearray(2048)
for inline in infile:
    if inline[0] == '#':
        pos = int(inline[3:6], 16) * 8
    elif len(inline) >= 8:
        dat = 0
        for i in range(0, 8):
            dat = (dat << 1)
            if inline[i] != '.':
                dat = dat | 1
        romdata[pos] = dat
        pos = pos + 1
outfile = open('font.rom', 'wb')
outfile.write(romdata)
outfile.close()
infile.close()
