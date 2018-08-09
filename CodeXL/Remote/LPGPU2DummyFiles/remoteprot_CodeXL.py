import socket
import sys
import struct
import time

#packet defines
PACKET_LENGTH = 8

# remote protocol tokens
CRP_ALIVE                   = 0
CRP_GET_DEVICE_INFO         = 1
CRP_GET_RAGENT_INFO         = 2
CRP_SEND_COLLECTION_OPTIONS = 3
CRP_START_COLLECTION        = 4
CRP_STOP_COLLECTION         = 5
CRP_FLUSH                   = 6
CRP_GET_SUPPORTED_APPS      = 7
CRP_CLEAN_STATE             = 8
CRP_DATA_PACKET             = 9

CRP_DISCONNECT              = 100
# response codes
CRP_RESPONSE_ACK            = 99
CRP_RESPONSE_NACK           = 98
CRP_RESPONSE_STOP           = 97

#canned packets

def SaveDataToFile(fname, data):
    with open(fname, 'wb') as theFile:
        theFile.write(data.encode())

def SendACKWithData(s, data):
    if len(data) == 0:
        SendCommand(s, CRP_RESPONSE_ACK)
    else:
        SendCommand(s, CRP_RESPONSE_ACK, len(data.encode()), data.encode())

def SendCommandWithData(s, CommandID, data):
    if len(data) == 0:
        SendCommand(s, CommandID)
    else:
        SendCommand(s, CommandID, len(data.encode()), data.encode())

def SendACKWithDataFromFile(s, fname):
    with open(fname, 'rb') as theFile:
        contents = theFile.read()
        if str(type(contents)) == "<class 'bytes'>":
            SendCommand(s, CRP_RESPONSE_ACK, len(contents), contents)
        else:
            SendCommand(s, CRP_RESPONSE_ACK, len(contents.encode()), contents.encode())

def SendCommandWithDataFromFile(s, CmdID, fname):
    with open(fname, 'rb') as theFile:
        contents = theFile.read()
        if str(type(contents)) == "<class 'bytes'>":
            SendCommand(s, CmdID, len(contents), contents)
        else:
            SendCommand(s, CmdID, len(contents.encode()), contents.encode())

def ReadDataWithLen(s, datalen, bSendACK=False, bKillXfer=False):
    edata = b''
    if datalen > 0:
        print ('datalen = ' + str(datalen))
        recvdatalen = 0
        data = b''
        while recvdatalen < datalen:
            tdata = s.recv(16)
            recvdatalen += len(tdata)
            data = data + tdata
        #edata = data.decode()
        edata = data
    if bSendACK == True:
        if bKillXfer == False:
            SendACK(s)
        else:
            SendStop(s)

    return edata

def SendACK(s):
    SendCommand(s, CRP_RESPONSE_ACK)

def SendStop(s):
    SendCommand(s, CRP_RESPONSE_STOP)

def SendNACK(s):
    SendCommand(s, CRP_RESPONSE_NACK)

def SendCommand(s, ID, dlen = 0, data = None):
    packet = struct.pack('>ii', ID, dlen)
    print('Packing command ' + str(ID) + ' ' + str(packet))
    s.sendall(packet)
    if dlen > 0:
        s.sendall(data)

def SendCommandExpectResponse(s, ID, RespID):
    extradatalen = 0
    extradata = None
    packet = struct.pack('>ii', ID, 0)
    print('Packing command ' + str(ID) + ' ' + str(packet))
    s.sendall(packet)
    response = s.recv(PACKET_LENGTH)
    if len(response) == PACKET_LENGTH:
        print('Got command with correct packet length')
        responseCode, extradatalen = struct.unpack('>ii', response)
        if extradatalen > 0:
            print('extra data present')
            recvdatalen = 0
            data = b''
            while recvdatalen < extradatalen:
                tdata = s.recv(16)
                recvdatalen += len(tdata)
                data = data + tdata
            extradata = data.decode()
    else:
        print('bad packet length = ' + str(len(response)))
        responseCode = -1

    if responseCode == RespID:
        rv = True
    else:
        rv = False

    return rv, responseCode, response, extradatalen, extradata

def ReadResponse(s, expectedCode):
    rv = False
    responseCode = -1
    extradatalen = 0

    response = s.recv(PACKET_LENGTH)
    if len(response) == PACKET_LENGTH:
        responseCode, extradatalen = struct.unpack('>ii', response)
        if responseCode == expectedCode:
            rv = True

    return rv, responseCode, extradatalen, response


def DeviceIsAlive(s):
    alive = False
    # First, is the device there
    print('Sending are you alive command')
    # First, is the device there
    while alive == False:
        rv, rcID, rawpacket, rvdata, extradatalen = SendCommandExpectResponse(sock, CRP_ALIVE, CRP_RESPONSE_ACK)
        # print(rv, rcID, rawpacket, rvdata, extradatalen)
        alive = rv

        if alive == False:
            print('Unexpected error: ', rv, rcID, rvdata)
        else:
            print('Remote device is alive!')
    return alive

bSeenHeader = False
numCounters = 0

def DecodeDCAPIv1Data(chunkFlags, chunkSize, data):
    global bSeenHeader
    global numCounters
    counters = []
    offset = 0

    if chunkFlags == 0:
        # print('Datalen = ' + str(len(data)))
        if bSeenHeader == False:
            magic, numCounters = struct.unpack('<II', data[:8])
            print('Magic = ' + str(hex(magic)) + ' numCounters = ' + str(numCounters))
            bSeenHeader = True
            offset = 8
        else:
            print('Seen header: ', str(len(data)))
        s = '<I' + ('i' * numCounters)
        print('chunk data len ' + str(len(data[offset:])))
        fields = struct.unpack(s, data[offset:])
        time = fields[0]
        print('Sample Time: ', str(time))
        for x in range(1, numCounters + 1):
            print ('    Counter ' + str(x) + ': ' + str(fields[x]))
    else:
        print ('Compressed chunks are not currently supported')

def DecodeDCAPIv2Data(chunkFlags, chunkSize, data):
    global numCounters
    counters = []
    offset = 0

    if chunkFlags == 0:
        print('Datalen = ' + str(len(data)))
        time, numCounters = struct.unpack('<QI', data[:12])
        print('Time = ' + str(time) + ' numCounters = ' + str(numCounters))
        offset = 12
        s = '<' + ('II' * numCounters)
        print('chunk data len ' + str(len(data[offset:])))
        fields = struct.unpack(s, data[offset:])
        print('Sample Time: ', str(time))
        fieldOffset = 0
        for x in range(0, numCounters):
            print('    Counter ' + str(x) + ' (ID=' + str(fields[fieldOffset]) + ') : ' + str(fields[fieldOffset+1]))
            fieldOffset = fieldOffset + 2
    else:
        print ('Compressed chunks are not currently supported')

def DecodeDCAPIv2Data(chunkFlags, chunkSize, data):

    if chunkFlags == 0:
        print('Datalen = ' + str(len(data)))
        time, numCounters = struct.unpack('<QI', data[:12])
        print('Time = ' + str(time) + ' numCounters = ' + str(numCounters))
        offset = 12
        s = '<' + ('II' * numCounters)
        print('chunk data len ' + str(len(data[offset:])))
        fields = struct.unpack(s, data[offset:])
        print('Sample Time: ', str(time))
        fieldOffset = 0
        for x in range(0, numCounters):
            print('    Counter ' + str(x) + ' (ID=' + str(fields[fieldOffset]) + ') : ' + str(fields[fieldOffset+1]))
            fieldOffset = fieldOffset + 2
    else:
        print ('Compressed chunks are not currently supported')

def DecodeTraceParams(chunkFlags, chunkSize, data):
    if chunkFlags == 0:
        print('TraceParams(Datalen) = ' + str(len(data)))
        frameNumber, drawNumber, threadId, apiId, cmdId, error, hasret = struct.unpack('<QQQIIiI', data[:40])
        print('fn = ' + str(frameNumber) + ' drawNumber = ' + str(drawNumber) +
                ' threadid = ' + str(threadId) + ' apiId = ' + str(apiId) +
                ' cmdId = ' + str(cmdId) + ' hasret = ' + str(hasret) )
    else:
        print ('Compressed chunks are not currently supported')


def DecodeTimeline(chunkFlags, chunkSize, data):
    if chunkFlags == 0:
        print('Timeline(Datalen) = ' + str(len(data)))
        trace_id, cpu_start, cpu_end, frame_num, draw_num, api_id, parameter_offset, parameter_length = struct.unpack('<QQQQQQQQ', data[:64])
        print('traceid = ' + str(trace_id) + ' cpustart = ' + str(cpu_start) +
                ' cpuend = ' + str(cpu_end) + ' fn = ' + str(frame_num) +
                ' drawnum = ' + str(draw_num) + ' apiid = ' + str(api_id))
    else:
        print ('Compressed chunks are not currently supported')

def DecodeShader(chunkFlags, chunkSize, data):
    if chunkFlags == 0:
        print('Shader(Datalen) = ' + str(len(data)))

        frame, draw, api, stype, cputime, shader_len = struct.unpack('<QQQQQQ', data[:6*8])
        shader_text = data[6*8:]
        print('frame = ' + str(frame) + ' draw = ' + str(draw) +
                ' api = ' + str(api) + ' type = ' + str(stype) +
                ' cputime = ' + str(cputime) + ' shader_len = ' + str(shader_len))
        print('Shader [' + shader_text + ']')
        #sys._exit()
    else:
        print ('Compressed chunks are not currently supported')

def DecodeGPUTiming(chunkFlags, chunkSize, data):
    if chunkFlags == 0:
        print('GPUTiming(Datalen) = ' + str(len(data)))

        timer_type, frame_num, draw_num, time = struct.unpack('<QQQQ', data[:4*8])

        print('timer type = ' + str(timer_type) + ' frame#: ' + str(frame_num) + ' draw#:' + str(draw_num) + ' time: ' + str(time))

        #sys._exit()
    else:
        print ('Compressed chunks are not currently supported')

def DecodeDataPacket(dp):
    print(len(dp))
    if dp[:4] == 'DCCH':
        print('Valid header found')
        chunkID, chunkFlags, chunkSize = struct.unpack('>III', dp[4:16])
        print('Chunk ID    : ', hex(chunkID))
        print('Chunk Flags : ', hex(chunkFlags))
        print('Chunk Size  : ', chunkSize)

        if chunkID == 0x3001:
            print('Recv packet length: ' + str(len(dp)))
            DecodeDCAPIv1Data(chunkFlags, chunkSize, dp[16:])
        elif chunkID == 0x3003:
            print('Recv packet length: ' + str(len(dp)))
            DecodeTraceParams(chunkFlags, chunkSize, dp[16:])
        elif chunkID == 0x3000:
            print('Recv packet length: ' + str(len(dp)))
            DecodeTimeline(chunkFlags, chunkSize, dp[16:])
        elif chunkID == 0x300B:
            print('Recv packet length: ' + str(len(dp)))
            DecodeDCAPIv2Data(chunkFlags, chunkSize, dp[16:])
        elif chunkID == 0x300A:
            print('Recv packet length: ' + str(len(dp)))
            DecodeShader(chunkFlags, chunkSize, dp[16:])
        elif chunkID == 0x3007:
            print('Recv packet length: ' + str(len(dp)))
            DecodeGPUTiming(chunkFlags, chunkSize, dp[16:])
        else:
            print('Unsupported chunk ID: ' + str(chunkID))

    else:
        print ('Invalid DP header: ' + dp[:4])
    pass

def Connect():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('', 0x5555)

    sock.connect(server_address)
    return sock


def Disconnect(sock):
    sock.close()


alive = False
haveDeviceName = False
haveDeviceinfo = False
sentCollectionInfo = False
amCollecting = False
haveFlushed = False
gotRAgentInfo = False

try:
    sock = Connect()

    print ('CXL Remote Protocol (CodeXL) - Started')

    # establish connection to android device
    while DeviceIsAlive(sock) == False:
       pass

    #get ragent info
    rv, rcID, rawpacket, datalen, data = SendCommandExpectResponse(sock, CRP_GET_RAGENT_INFO, CRP_RESPONSE_ACK)
    if rv == True:
#        apps = data.split(':')
#        print('Ragent info: ', str(apps))
        gotRAgentInfo = True
        SaveDataToFile('.//TargetDefinition.xml', data)
    else:
        print('Error Getting ragent info: ', rv, rcID, extradatalen, response)


    # get device info
    if gotRAgentInfo:
        rv, rcID, rawpacket, datalen, data = SendCommandExpectResponse(sock, CRP_GET_DEVICE_INFO, CRP_RESPONSE_ACK)
        if rv == True:
            if datalen < 40:
                print('Device Info: [' + str(data) + ']')
            else:
                print('Device Info: [too long to print]')
            haveDeviceinfo = True
            SaveDataToFile('.//TargetCharacteristics.xml', data)
        else:
            print('Error getting device info:', rv, rcID, data)


    #update collection definitions from GUI and then....
    if haveDeviceinfo:
        SendCommandWithDataFromFile(sock, CRP_SEND_COLLECTION_OPTIONS, './/CollectionDefinition.xml')
        rv, rcID, extradatalen, response = ReadResponse(sock, CRP_RESPONSE_ACK)
        if rv == True:
            #rv, rcID, extradatalen, response = ReadResponse(sock, CRP_RESPONSE_NACK)
            #print('RV is ' + str(rv))
            print('Sent collection definition')
            sentCollectionInfo = True
        else:
            print('Error sending collection definition:', rv, rcID, extradatalen, response)

    # get supported apps
    if sentCollectionInfo:
        rv, rcID, rawpacket, datalen, data = SendCommandExpectResponse(sock, CRP_GET_SUPPORTED_APPS, CRP_RESPONSE_ACK)
        if rv == True:
            apps = data.split(':')
            print('Supported Apps: ', str(apps))
            gotApps = True
        else:
            print('Error Getting supported apps: ', rv, rcID, extradatalen, response)

    # user chooses app to profile
    if gotApps:
        tgtApp = apps[0]
        print('App: ' + tgtApp + ' chosen for collection')
        SendCommandWithData(sock, CRP_START_COLLECTION, tgtApp)
        rv, rcID, extradatalen, response = ReadResponse(sock, CRP_RESPONSE_ACK)
        if rv == True:
            print('Collection Started')
            amCollecting = True
        else:
            print('Error starting collection: ', rv, rcID, extradatalen, response)


    #receive some data packets
    counter = 0
    haveMoreData = True
    if amCollecting == True:
        while haveMoreData == True:
            rv, rcID, extradatalen, response = ReadResponse(sock, CRP_DATA_PACKET)
            print ("Counter = " + str(counter))
            if rv == True:
                if extradatalen > 0:
                    if counter <= 10000000:
                        datap = ReadDataWithLen(sock, extradatalen, True)
                    else:
                        datap = ReadDataWithLen(sock, extradatalen, True, True)
                        print('send kill due to counter value')
                        haveMoreData = False
                    print('got data packet: [' + str(datap) + ']')
                    DecodeDataPacket(datap)
                else:
                    print('got empty data packet. terminating collection', rv, rcID, extradatalen, response)
                    haveMoreData = False
                counter = counter + 1
            else:
                print('data packet read failed. exiting collection mode', rv, rcID, extradatalen, response)
                haveMoreData = False

    # stop collection
    if amCollecting:
        SendCommand(sock, CRP_STOP_COLLECTION)
        rv, rcID, extradatalen, response = ReadResponse(sock, CRP_RESPONSE_ACK)
        if rv == True:
            print('Collection Stopped')
            amCollecting = False
        else:
            print('Error stopping collection: ', rv, rcID, extradatalen, response)

    # flush data
    if amCollecting == False:
        SendCommand(sock, CRP_FLUSH)
        rv, rcID, extradatalen, response = ReadResponse(sock, CRP_RESPONSE_ACK)
        if rv == True:
            print('Data flushed')
            haveFlushed = True
        else:
            print('Error flushing data: ', rv, rcID, extradatalen, response)

    # clean state
    if haveFlushed == True:
        SendCommand(sock, CRP_CLEAN_STATE)
        rv, rcID, extradatalen, response = ReadResponse(sock, CRP_RESPONSE_ACK)
        if rv == True:
            print('State cleaned')
            haveCleaned = True
        else:
            print('Error cleaning state: ', rv, rcID, extradatalen, response)

    # and disconnect from service
    SendCommand(sock, CRP_DISCONNECT)

    print ('all done')
finally:
    Disconnect(sock)

