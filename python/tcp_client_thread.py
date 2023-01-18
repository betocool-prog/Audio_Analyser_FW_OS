#!/usr/bin/env python

import socket
import numpy as np
from PyQt5.QtCore import QThread, pyqtSignal


class TCPClientThread(QThread):

    rx_complete_signal = pyqtSignal()

    def __init__(self):
        QThread.__init__(self)
        self.LOCAL_IP = "192.168.2.144"
        self.LOCAL_PORT = 8888
        self.sample_type = np.dtype([('left', np.int32), ('right', np.int32)])
        self.left_samples = None
        self.right_samples = None
        self.sock = None
        self.connected = False
        self.len_expected = 0

    def rx_complete(self):
        self.rx_complete_signal.emit()

    def set_expected_len(self, length):
        self.len_expected = length

    def run(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            # Establish connection to TCP server and exchange data
            self.sock.bind(("192.168.2.20", 0))
            self.sock.connect((self.LOCAL_IP, self.LOCAL_PORT))
            self.connected = True

        except Exception as e:
            print(f"Could not connect: {repr(e)}")
            self.connected = False

        if self.connected:

            total_len = 0
            total_data = bytearray()
            rx_data_flag = True

            while rx_data_flag:

                try:
                    # print("Waiting for message...")
                    data = self.sock.recv(1500)

                    if not data:
                        rx_data_flag = False
                    else:
                        total_data.extend(data)
                        total_len += len(data)

                except socket.timeout as e:
                    print("Socket timeout: {}".format(e))
                    total_len = 0
                    total_data = bytearray()
                    rx_data_flag = False

                if total_len >= self.len_expected:
                    # print("Total Len: {}".format(total_len))
                    samples = np.frombuffer(total_data, dtype=self.sample_type)
                    total_len = 0
                    total_data = bytearray()

                    self.left_samples = (np.int32(samples['left'] << 8))
                    self.right_samples = (np.int32(samples['right'] << 8))
                    self.rx_complete()
                    rx_data_flag = False
                    self.sock.close()

    def stop(self):
        self.terminate()


if __name__ == "__main__":
    pass
