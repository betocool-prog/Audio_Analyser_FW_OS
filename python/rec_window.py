#!/usr/bin/env python3

from PyQt5.QtCore import pyqtSlot, QThread
from PyQt5 import QtWidgets
from pathlib import Path
import numpy as np
import socket
import datetime as dt


class TCPClientThread(QThread):

    def __init__(self):
        QThread.__init__(self)
        self.LOCAL_IP = "192.168.1.144"
        self.LOCAL_PORT = 8888
        self.sample_type = np.dtype([('left', np.int32), ('right', np.int32)])
        self.sock = None
        self.connected = False
        self.raw_samples = None
        self.rx_data_flag = True
        self.curr_file = None

    def set_file(self, file_ptr):
        self.curr_file = file_ptr

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
            self.rx_data_flag = True

            while self.rx_data_flag:

                try:
                    data = self.sock.recv(1500)

                    if not data:
                        self.rx_data_flag = False
                    else:
                        total_data.extend(data)
                        total_len += len(data)

                except socket.timeout as e:
                    print("Socket timeout: {}".format(e))
                    total_len = 0
                    total_data = bytearray()
                    self.rx_data_flag = False

                if total_len >= (4096 * 8):
                    self.raw_samples = np.int32(np.frombuffer(total_data, dtype=np.int32) << 8)
                    total_len = 0
                    total_data = bytearray()
                    self.curr_file.write(self.raw_samples.tobytes())

            self.sock.close()

    def stop(self):
        # self.terminate()
        self.rx_data_flag = False


class RecWindow(QtWidgets.QMainWindow):

    def __init__(self):
        super().__init__()
        self.statusBar()
        self.main_menu = self.menuBar()
        self.file_menu = self.main_menu.addMenu('&File')

        self.open_folder_action = QtWidgets.QAction("Open Folder...", self)
        self.file_menu.addAction(self.open_folder_action)

        # Central widget to add layouts
        # Set the central widget for the main window - Vertical layout
        super().setCentralWidget(QtWidgets.QWidget())
        super().centralWidget().setLayout(QtWidgets.QVBoxLayout())

        # Make the layout available to add widgets
        self.main_layout = super().centralWidget().layout()

        self.buttons = {}
        self.textedit = QtWidgets.QTextEdit()
        self.current_folder = str(Path.home()) + "/Music"

        self.tcp_client = None
        self.current_file = None

    def build_window(self):
        self.main_layout.addWidget(self.textedit)

        # Buttons Horizontal Layout
        layout = QtWidgets.QHBoxLayout()
        self.buttons["Start"] = QtWidgets.QPushButton("Start")
        self.buttons["Stop"] = QtWidgets.QPushButton("Stop")
        self.buttons["New File"] = QtWidgets.QPushButton("New File")
        layout.addWidget(self.buttons["Start"])
        layout.addWidget(self.buttons["Stop"])
        layout.addWidget(self.buttons["New File"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.main_layout.addLayout(layout)

        # Resize and rename window
        super().setWindowTitle("Audio Recorder Window")

        temp_text = "Current Folder: {}".format(self.current_folder)
        self.textedit.append(temp_text)

        # Connect signals and slots
        self.open_folder_action.triggered.connect(self.open_folder)
        self.buttons["Start"].clicked.connect(self.start_logging)
        self.buttons["Stop"].clicked.connect(self.stop_logging)
        self.buttons["New File"].clicked.connect(self.stop_logging)

        self.tcp_client = TCPClientThread()

    @pyqtSlot()
    def open_folder(self):
        temp_dir = QtWidgets.QFileDialog.getExistingDirectory(
            self, 'Open Directory', self.current_folder)

        if temp_dir != "":
            self.current_folder = temp_dir
            temp_text = "Current Folder: {}".format(temp_dir)
            self.textedit.setText(temp_text)

    @pyqtSlot()
    def start_logging(self):
        now_string = dt.datetime.now().strftime("%y%m%dT%H%M%S")
        full_path = self.current_folder + "/" + now_string + ".wav"
        self.current_file = open(full_path, 'wb')
        self.tcp_client.set_file(self.current_file)
        self.textedit.append("Started file: {}".format(full_path))
        self.tcp_client.start()

    @pyqtSlot()
    def stop_logging(self):
        self.tcp_client.stop()
        self.current_file.close()
        self.textedit.append("Stop logging")

    @pyqtSlot()
    def new_file(self):
        pass
