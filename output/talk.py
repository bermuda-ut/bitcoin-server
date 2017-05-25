import socket
import random
import fcntl, os

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


server = 'localhost'
port = 1234
s.connect((server, port))
fcntl.fcntl(s, fcntl.F_SETFL, os.O_NONBLOCK)

messages = ['WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212000 01\r\n',
'SOLN 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212147\r\n',
'WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 01\r\n',
'SOLN 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212605\r\n',
'WORK 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 04\r\n',
'SOLN 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 100000002321ed8f\r\n',
'WORK 1dffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 01\r\n',
'SOLN 1dffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023f6c072\r\n',
'WORK 1d29ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 04\r\n',
'SOLN 1d29ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000026b9c904\r\n']

messages += messages
messages += messages
messages += messages
messages += messages
messages += messages
messages += messages
messages += messages
messages += messages
messages += messages
messages += messages
messages += messages
messages += messages
messages += messages

for i in messages:
    try:
        s.send(i.encode())
        s.send(b'PING\r\n')
        if random.random() > 0.85:
            s.send(b'PING\r\n')
            s.send(b'ABRT\r\n')
            s.send(b'PING\r\n')

        if random.random() > 0.9999:
            s.send(b'PING\r\n')
            s.close()
            s.connect((server, port))
        try:
            a = s.recv(2048)
        except socket.error:
            pass
    except OSError:
        pass
    # print(a)
import time
time.sleep(20)
s.close()
