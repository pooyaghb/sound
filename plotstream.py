import socket
import time
import struct 
import matplotlib.pyplot as plt
import numpy as np

channels = 2
chunk    = 256
duration = 5333 * 187
du_tmp   = 5333
N        = int(duration/du_tmp)
LEN      = N * chunk

clientport   = 12500
serverport   = 11500
clientaddr   = ("192.168.1.152", clientport)
clientsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
clientsocket.bind(clientaddr)
print(f"Listening on {clientaddr}")

x       = np.arange(0, LEN, 1)
data    = np.zeros((LEN * channels,), dtype = np.int16)
datach1 = np.zeros((LEN,), dtype = np.int16)
datach2 = np.zeros((LEN,), dtype = np.int16)

fig, ax = plt.subplots(2, 1)
ax[0].set_ylim(-1*pow(2, 15), pow(2, 15))
ax[1].set_ylim(-1*pow(2, 15), pow(2, 15))
line1, = ax[0].plot(x[0:-2], datach1[0:-2])
line2, = ax[1].plot(x[0:-2], datach2[0:-2])
ax[0].grid(True)
ax[1].grid(True)
plt.show(block = False)


while True:
    
    message = clientsocket.recvfrom(2 * chunk * channels + 1)[0]  
    
    if(message[0] == 255):
        data[0:chunk*channels] = struct.unpack(f"<{chunk * channels}h", message[1:])

        for i in range(N-1):
            message = clientsocket.recvfrom(2 * chunk * channels + 1)[0]
            data[(i+1)*chunk*channels:(i+2)*chunk*channels] = struct.unpack(f"<{chunk * channels}h", message[1:])
            
            
        #print(message[0])
    
        datach1 = data[::2]
        datach2 = data[1::2]

        line1.set_ydata(datach1[0:-2])
        line2.set_ydata(datach2[0:-2])
        fig.canvas.draw()
        fig.canvas.flush_events()
    
    else:
        continue





