import bluetooth
import time
import copy


class MessageProtocol:
    def __init__(self, op, cmd, ctrl, data):
        """
        :param op:
        opcode of message 0: GET, 1: SET
        :param cmd:
        detailed settings of operation.
        GET: 0: temp, 1: humidiry
        SET: 0: LED, 1: LCD
        :param ctrl:
        notation bit for SET LCD
        else, reserved
        :param data:
        Other datas.
        """
        self.opcode = op
        self.cmd = cmd
        self.notation = ctrl
        self.data = data
        self.message = self.build_protocol()

    def build_protocol(self):
        header_bit_string = str(0) * 2 + str(1) * 2 + str(0) + str(self.opcode) + str(self.cmd) + str(self.notation)
        header_bit = int(header_bit_string, 2)
        header_bytes = bytes([header_bit])
        data_bytes = bytes()
        for ind_data in self.data:
            if type(ind_data) is str:
                ind_data_bytes = ind_data.encode()
                data_bytes += ind_data_bytes
            elif type(ind_data) is int:
                ind_data_bytes = bytes([ind_data])
                data_bytes += ind_data_bytes
        end_bytes = b"\xde\xad"
        return header_bytes + data_bytes + end_bytes


def get_address():
    nearby_devices = bluetooth.discover_devices()
    res_addr = None
    for bdaddr in nearby_devices:
        device_name = bluetooth.lookup_name(bdaddr)
        if device_name == "H-C-2010-06-01":
            res_addr = bdaddr
    return res_addr


def get_data(bdaddr):
    res_data = list()
    for i in range(0, 2):
        message = MessageProtocol(0, i, 0, [])
        recv_bytes = send_message(bdaddr, message.message)
        recv_protocol = decode_recv(recv_bytes)
        print(recv_protocol)
        if type(recv_protocol) is str:
            return "error occured"
        res_data.append(recv_protocol.data)
        if i == 0:
            time.sleep(5)
    return res_data


def send_led(bdaddr, list_threshold):
    message = MessageProtocol(1, 0, 0, list_threshold)
    recv_bytes = send_message(bdaddr, message.message)
    recv_protocol = decode_recv(recv_bytes)
    if type(recv_protocol) is str:
        return recv_protocol
    else:
        return 0


def send_lcd(bdaddr, cmd, output):
    message = MessageProtocol(1, 1, cmd, [output])
    recv_bytes = send_message(bdaddr, message.message)
    recv_protocol = decode_recv(recv_bytes)
    if type(recv_protocol) is str:
        return recv_protocol
    else:
        return 0


def send_message(address, data):
    blesocket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    try:
        blesocket.connect((address, 1))
        print("connection complete! send message!")
        print(data)
        blesocket.send(data)
        recv_buffer = bytes()
        recv_complete = True
        while recv_complete:
            recv_data = blesocket.recv(1024)
            print("recv_data: %s" % recv_data)
            if b'\xde\xad' in recv_data:
                copy_recv_data = copy.deepcopy(recv_data)
                end_byte = 0
                for i in range(len(copy_recv_data)):
                    if copy_recv_data[i] == 222:
                        end_byte = i
                        recv_complete = False
                        break
                recv_buffer += recv_data[:end_byte]
            else:
                recv_buffer += recv_data
        blesocket.close()
        print("recv_buffer: " + str(recv_buffer))
        return recv_buffer
    except bluetooth.btcommon.BluetoothError as err:
        print("An error occured: %s" % err)
        if "112" in str(err):
            res_resend = send_message(address, data)
            return res_resend
        else:
            return "error detected!"


def decode_recv(data):
    print(data)
    if type(data) is not bytes:
        return "error detected"
    protocol = MessageProtocol(0, 0, 0, [])
    header_bytes = data[0]
    header_bin = bin(header_bytes)
    print("header_bin: %s" % header_bin)
    if header_bin[4] == 0:
        print("Error! Why this packet is not ACK?")
        return "NON ACK Error"
    protocol.opcode = int(header_bin[5])
    protocol.cmd = int(header_bin[6])
    protocol.notation = int(header_bin[7])
    protocol.data = list()
    if len(data) > 1:
        protocol.data.append(data[1:].decode())
    return protocol
