from PyQt5.QtCore import pyqtSlot, pyqtSignal, QThread
from PyQt5 import QtWidgets
import pyqtgraph as pg
from pathlib import Path
import numpy as np
import socket
import datetime as dt
import sys


class TCPClientThread(QThread):

    rx_complete_signal = pyqtSignal()

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
        self.new_file = False
        self.data_array = bytearray()

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
                        self.data_array.extend(data)

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
                    self.rx_complete_signal.emit()

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

        pg.setConfigOption('background', 'w')
        self.plot_widgets = {}

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

        # VU plot widgets
        # layout = QtWidgets.QHBoxLayout()
        # self.plot_widgets["Left"] = pg.PlotWidget()
        # self.plot_widgets["Right"] = pg.PlotWidget()
        #
        # for key in self.plot_widgets:
        #     layout.addWidget((self.plot_widgets[key]))
        #     # layout.addWidget((self.plot_widgets["Right"]))
        #
        # self.plot_bg()
        #
        # self.main_layout.addLayout(layout)

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
        # self.tcp_client.rx_complete_signal.connect(self.plot)

    # def plot_bg(self):
    #
    #     for key in self.plot_widgets:
    #         plot_w = self.plot_widgets[key]
    #         plot_w.hideAxis("left")
    #         plot_w.hideAxis("bottom")
    #         plot_w.setAspectLocked()
    #         plot_w.setXRange(-1.2, 1.2)
    #         plot_w.setYRange(0, 1.2)
    #         r = 0.1
    #         circle = pg.QtGui.QGraphicsEllipseItem(-r, -r, 2*r, 2*r)
    #         circle.setStartAngle(210 * 16)
    #         circle.setSpanAngle(120 * 16)
    #         circle.setPen(pg.mkPen(1))
    #         plot_w.addItem(circle)
    #
    #         r = 1.2
    #         circle = pg.QtGui.QGraphicsEllipseItem(-r, -r, 2*r, 2*r)
    #         circle.setStartAngle(210 * 16)
    #         circle.setSpanAngle(120 * 16)
    #         circle.setPen(pg.mkPen(0.2))
    #         plot_w.addItem(circle)

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

    # @pyqtSlot()
    # def plot(self):
    #     if len(self.tcp_client.data_array) >= (4096 * 8):
    #         temp_data = self.tcp_client.data_array[0:(4096 * 8)]
    #         self.tcp_client.data_array = self.tcp_client.data_array[(4096 * 8):]
    #         plot_data = np.int64(np.frombuffer(temp_data, dtype=np.int32) << 8)
    #         left = plot_data[0::2]
    #         right = plot_data[1::2]
    #         left_rms = np.sqrt(np.mean(left**2))
    #         right_rms = np.sqrt(np.mean(right**2))
    #         left_rms_db = 20 * np.log10(left_rms / (2 ** 31))
    #         right_rms_db = 20 * np.log10(right_rms / (2 ** 31))
    #         left_angle = 60 + 120 * left_rms_db/93
    #         right_angle = 60 + 120 * right_rms_db/93
    #         l_x1 = 0.1 * np.sin(left_angle * 2 * np.pi / 360)
    #         l_x2 = 1.2 * np.sin(left_angle * 2 * np.pi / 360)
    #         l_y1 = 0.1 * np.cos(left_angle * 2 * np.pi / 360)
    #         l_y2 = 1.2 * np.cos(left_angle * 2 * np.pi / 360)
    #         r_x1 = 0.1 * np.sin(right_angle * 2 * np.pi / 360)
    #         r_x2 = 1.2 * np.sin(right_angle * 2 * np.pi / 360)
    #         r_y1 = 0.1 * np.cos(right_angle * 2 * np.pi / 360)
    #         r_y2 = 1.2 * np.cos(right_angle * 2 * np.pi / 360)
    #
    #         self.plot_widgets["Left"].plot([l_x1, l_x2], [l_y1, l_y2], pen="r", clear=True)
    #         self.plot_widgets["Right"].plot([r_x1, r_x2], [r_y1, r_y2], pen="r", clear=True)


if __name__ == "__main__":

    # Create the main application instance
    app = QtWidgets.QApplication([])

    rec_window = RecWindow()
    rec_window.build_window()
    rec_window.show()

    # Gracefully exit the application
    sys.exit(app.exec_())
