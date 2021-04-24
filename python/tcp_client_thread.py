#!/usr/bin/env python

import socket
import numpy as np
from PyQt5.QtCore import QThread, pyqtSignal


class TCPClientThread(QThread):

    rx_complete_signal = pyqtSignal()

    def __init__(self):
        QThread.__init__(self)
        self.LOCAL_IP = "192.168.1.144"
        self.LOCAL_PORT = 8888
        self.sample_type = np.dtype([('left', np.int32), ('right', np.int32)])
        self.left_samples = None
        self.right_samples = None
        self.sock = None
        self.connected = False

    def rx_complete(self):
        self.rx_complete_signal.emit()

    def run(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            # Establish connection to TCP server and exchange data
            self.sock.connect((self.LOCAL_IP, self.LOCAL_PORT))
            self.connected = True

        except Exception as e:
            print("Could not connect")
            self.connected = False

        if self.connected:

            total_len = 0
            total_data = bytearray()
            rx_data_flag = True
            # self.sock.settimeout(0.2)

            while rx_data_flag:

                try:
                    # print("Waiting for message...")
                    data = self.sock.recv(1500)
                    if not data:
                        rx_data_flag = False
                    else:
                        print("Received: {}".format(len(data)))
                        total_data.extend(data)
                        total_len += len(data)

                except socket.timeout as e:
                    print("Socket timeout: {}".format(e))
                    total_len = 0
                    total_data = bytearray()
                    rx_data_flag = False

            print("Total length received: {} bytes".format(total_len))
            # # print("Received: {}".format(total_len))
            # total_data_bytes = bytes(total_data)
            #
            # if total_len >= self.nr_of_bytes_expected:
            #     # print("Total Len: {}".format(total_len))
            #     total_len = 0
            #     total_data = bytearray()
            #     # print("Data type: {}".format(type(data)))
            #     # print("Total_data type: {}".format(type(total_data_bytes)))
            #     samples = np.frombuffer(total_data_bytes, dtype=self.sample_type)
            #
            #     self.left_samples = (np.int32(samples['left'] << 8))
            #     self.right_samples = (np.int32(samples['right'] << 8))
                # self.rx_complete()

    def stop(self):
        self.terminate()


if __name__ == "__main__":
    pass
