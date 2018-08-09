import socket
import sys
import struct
import time

CXL_REMOTE_PORT_ID          = 0x5555

#packet defines
PACKET_LENGTH = 8

# remote protocol tokens
CRP_ALIVE                   = 0
CRP_GET_DEVICE_INFO         = 1
CRP_GET_DEVICE_NAME         = 2
CRP_SEND_COLLECTION_OPTIONS = 3
CRP_START_COLLECTION        = 4
CRP_STOP_COLLECTION         = 5
CRP_FLUSH                   = 6
CRP_GET_SUPPORTED_APPS      = 7
CRP_CLEAN_STATE             = 8
CRP_DATA_PACKET             = 9

# response codes
CRP_RESPONSE_ACK            = 99
CRP_RESPONSE_NACK           = 98

#canned packets
aliveCommand = int(0x00)

def SendACKWithData(s, data):
    if len(data) == 0:
        SendCommand(s, CRP_RESPONSE_ACK)
    else:
        SendCommand(s, CRP_RESPONSE_ACK, len(data.encode()), data.encode())

def SendACKWithDataFromFile(s, fname):
    with open(fname, 'rb') as theFile:
        contents = theFile.read()
        SendCommand(s, CRP_RESPONSE_ACK, len(contents), contents)

def SendCommandWithData(s, ID, data):
    if len(data) == 0:
        SendCommand(s, ID)
    else:
        SendCommand(s, ID, len(data.encode()), data.encode())


def SendACK(s):
    SendCommand(s, CRP_RESPONSE_ACK)

def SendNACK(s):
    SendCommand(s, CRP_RESPONSE_NACK)

def SendCommand(s, ID, dlen = 0, data = None):
    packet = struct.pack('<ii', ID, dlen)
    print('Packing command ' + str(ID) + ' ' + str(packet))
    s.sendall(packet)
    if dlen > 0:
        s.sendall(data)


def RecvFile(s, datalen, filename):
    edata = b''
    if datalen > 0:
        print ('datalen = ' + str(datalen))
        recvdatalen = 0
        data = b''
        while recvdatalen < datalen:
            print ('receiving data')
            tdata = s.recv(16)
            recvdatalen += len(tdata)
            data = data + tdata
        edata = data

    with open(filename, 'wb') as theFile:
        theFile.write(edata)

    SendACK(s)

def ReadDataWithLen(s, datalen):
    edata = b''
    if datalen > 0:
        print ('datalen = ' + str(datalen))
        recvdatalen = 0
        data = b''
        while recvdatalen < datalen:
            tdata = s.recv(16)
            recvdatalen += len(tdata)
            data = data + tdata
        edata = data.decode()
    SendACK(s)

    return edata



sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', CXL_REMOTE_PORT_ID)
sock.bind(server_address)
print (server_address)
sock.listen(1)

alive = False

try:
    print ('CXL Remote Protocol Server (ANDROID) - Started')

    while True:
        conn, addr = sock.accept()
        print('Connection from ', addr)
    
        while True:
            print('Waiting for commands')
            aCommand = conn.recv(PACKET_LENGTH)
            if len(aCommand) == PACKET_LENGTH:
                commandID, datalen = struct.unpack('<ii', aCommand)
                print("received command ", commandID)
    
                #alive?
                if commandID == CRP_ALIVE:
                    print('Received ALIVE')
                    SendACK(conn)
                # get device name
                elif commandID == CRP_GET_DEVICE_NAME:
                    print('Received GetDeviceName')
                    SendACKWithData(conn, 'MySpinglyWingly')
                # get device info
                elif commandID == CRP_GET_DEVICE_INFO:
                    print('Received GetDeviceInfo')
                    SendACKWithDataFromFile(conn, './/TargetCharacteristics.xml')
                # send collection definition
                elif commandID == CRP_SEND_COLLECTION_OPTIONS:
                    print('Received SendCollectionOptions')
                    RecvFile(conn, datalen, './/CollectionDefinition.xml')
                # Get supported apps
                elif commandID == CRP_GET_SUPPORTED_APPS:
                    print('Received GetSupportedApps')
                    SendACKWithData(conn, 'com.android.test:com.pickles.pong:com.samsung.glbench')
                # start collection
                elif commandID == CRP_START_COLLECTION:
                    print('Received StartCollection')
                    SendACK(conn)
    
                    for x in range(1, 200):
                        print ('Sending data packet: ' + str(x))
                        SendCommandWithData(conn, CRP_DATA_PACKET, 'data packet ' + str(x) + ' blah blah')
                        time.sleep(0.1)
                    print('sending empty command to stop collection')
                    SendCommand(conn, CRP_DATA_PACKET)
                # stop collection
                elif commandID == CRP_STOP_COLLECTION:
                    print('Stop Collecting')
                    SendACK(conn)
                # flush state
                elif commandID == CRP_FLUSH:
                    print('Flush state')
                    SendACK(conn)
                # stop collection
                elif commandID == CRP_CLEAN_STATE:
                    print('clean state')
                    SendACK(conn)
                else:
                    print('Received Unsupported command id = ' + str(commandID))
            else:
                print('Data with invalid length received. Length = ' + str(len(aCommand)))
                break
    
        print("Disconnected")
    print ('all done')
finally:
    conn.close()
    sock.close()



