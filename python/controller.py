#!/usr/bin/env python3

from PyQt5.QtCore import pyqtSlot, QTimer, QObject

import client
import tcp_client_thread as tcp_c_t
import data_proc
import audio_plot_window as apw


class Controller(QObject):

    def __init__(self):

        super().__init__()

        self.signal_config = client.analyser_pb2.SignalConfig()
        self.signal_config.function = client.analyser_pb2.SINE
        self.signal_config.op_mode = client.analyser_pb2.SYNC
        self.signal_config.frequency = 1000
        self.signal_config.amplitude = 1.4 / 1.6
        self.signal_config.signal_preamble = 0
        self.signal_config.fft_size = 4096
        self.signal_config.signal_len = self.signal_config.fft_size
        self.signal_config.signal_end = 0
        self.sync_samples_max = 485

        self.timer = QTimer()

        self.tcp_client = tcp_c_t.TCPClientThread()
        self.tcp_client.rx_complete_signal.connect(self.data_rdy_slot)

        self.processor = data_proc.DataProc(self.signal_config)
        self.processor.set_data_source(self.tcp_client)

        self.main_window = apw.AudioPlotMainWindow()
        self.main_window.set_fft_graph(self.processor.grFFT)
        self.main_window.set_pcm_graph(self.processor.grPCM)
        self.main_window.build_window()

        # Connect signals and slots
        # Start / stop getting data
        self.timer.timeout.connect(self.timeout)
        self.main_window.buttons["free_running"].clicked.connect(self.get_free_running_clicked)
        self.main_window.buttons["single_shot"].clicked.connect(self.get_single_shot_clicked)
        # self.main_window.checkboxes["avg"].clicked.connect(self.avg_clicked)
        # self.main_window.checkboxes["x_log"].clicked.connect(self.xlog_clicked)
        # self.main_window.checkboxes["show_fft_left"].clicked.connect(self.fft_show_clicked)
        # self.main_window.checkboxes["show_fft_right"].clicked.connect(self.fft_show_clicked)
        #
        self.main_window.fft_rb_group.buttonClicked.connect(self.function_clicked)
        # self.main_window.pcm_rb_group.buttonClicked.connect(self.l_r_clicked)
        #
        self.main_window.line_edit["Frequency"].returnPressed.connect(self.frequency_changed)
        self.main_window.line_edit["Amplitude"].returnPressed.connect(self.amplitude_changed)
        # self.main_window.line_edit["Impedance"].returnPressed.connect(self.impedance_changed)
        #
        # self.main_window.drop_down["fft_windowing"].activated.connect(self.fft_windowing_changed)
        # self.main_window.drop_down["pcm_windowing"].activated.connect(self.pcm_windowing_changed)
        #
        # self.main_window.drop_down["pcm_windowing"].setCurrentIndex(0)
        # self.main_window.drop_down["fft_windowing"].setCurrentIndex(3)
        #
        # # Set default client settings
        client.Client().set_signal_config(self.signal_config)

    # Gets called whenever the TCP data has arrived
    @pyqtSlot()
    def data_rdy_slot(self):
        self.processor.data_rdy_slot()

        if self.signal_config.op_mode == getattr(client.analyser_pb2, 'FREE_RUNNING'):
            self.timer.setInterval(10)
            self.timer.setSingleShot(True)
            self.timer.start()

    # Gets called when the get data button is pressed
    @pyqtSlot()
    def get_free_running_clicked(self):
        self.signal_config.op_mode = getattr(client.analyser_pb2, 'FREE_RUNNING')
        client.Client().set_signal_config(self.signal_config)
        self.tcp_client.set_expected_len(self.signal_config.fft_size * 8)
        self.tcp_client.start()

    @pyqtSlot()
    def get_single_shot_clicked(self):
        freq = self.signal_config.frequency
        self.signal_config.op_mode = client.analyser_pb2.SYNC;
        # self.signal_config.function = client.analyser_pb2.SINE;
        self.signal_config.signal_preamble = 0;
        self.signal_config.signal_len = int(int(self.signal_config.fft_size * freq / 96000) * 96000 / freq)
        self.signal_config.signal_end = self.signal_config.fft_size - self.signal_config.signal_len
        client.Client().set_signal_config(self.signal_config)
        self.tcp_client.set_expected_len((self.sync_samples_max + self.signal_config.fft_size) * 8)
        self.tcp_client.start()

    # Gets called when the average button is pressed
    @pyqtSlot()
    def avg_clicked(self):

        if self.main_window.checkboxes["avg"].isChecked():
            self.processor.average = True

        else:
            self.processor.average = False

    # Gets called when the x_log button is pressed
    @pyqtSlot()
    def xlog_clicked(self):

        self.processor.grFFT.plotItem.setLogMode(x=self.main_window.checkboxes["x_log"].isChecked())
        self.processor.grFFT.plotItem.disableAutoRange()
        self.processor.grFFT.plotItem.setYRange(-160, 5, padding=0.02)

    # Gets called when the L/R radiobuttons are pressed
    @pyqtSlot()
    def l_r_clicked(self):
        for item in self.main_window.pcm_rb.items():
            value = item[1]
            if value.isChecked():
                self.processor.pcm_mode = value.text()

    # Gets called when the function radiobuttons are pressed
    @pyqtSlot()
    def function_clicked(self):

        for item in self.main_window.fft_rb.items():
            if item[1].isChecked():
                bt_name = item[1].text()
                break

        if bt_name == "Impulse":
            try:
                self.signal_config.function = client.analyser_pb2.IMPULSE
                client.Client().set_signal_config(self.signal_config)
            except Exception as e:
                print("Impulse Changed Exception: {}".format(repr(e)))

        if bt_name == "Sine":
            try:
                self.signal_config.function = client.analyser_pb2.SINE
                client.Client().set_signal_config(self.signal_config)
            except Exception as e:
                print("Sine Changed Exception: {}".format(repr(e)))

    # Gets called when the function radiobuttons are pressed
    @pyqtSlot()
    def impedance_changed(self):
        res_value = self.main_window.line_edit["Impedance"].text()
        if res_value.isdecimal():
            self.processor.resistor = float(res_value)

    # Gets called when the FFT show checkboxes are clicked
    @pyqtSlot()
    def fft_show_clicked(self):
        self.processor.show_fft_left = self.main_window.checkboxes["show_fft_left"].isChecked()
        self.processor.show_fft_right = self.main_window.checkboxes["show_fft_right"].isChecked()

    # Gets called when the frequency changes
    @pyqtSlot()
    def frequency_changed(self):
        freq_val = self.main_window.line_edit["Frequency"].text()
        try:
            self.signal_config.frequency = int(freq_val)
            client.Client().set_signal_config(self.signal_config)
        except Exception as e:
            print("Frequency Changed Exception: {}".format(repr(e)))

    # Gets called when the amplitude changes
    @pyqtSlot()
    def amplitude_changed(self):
        amp_val = self.main_window.line_edit["Amplitude"].text()
        try:
            self.signal_config.amplitude = float(amp_val) / 1.6
            client.Client().set_signal_config(self.signal_config)
        except Exception as e:
            print("Amplitude Changed Exception: {}".format(repr(e)))

    # Gets called when the fft windowing function changes
    @pyqtSlot(int)
    def fft_windowing_changed(self, index):
        print("Val: {}".format(index))
        self.processor.fft_windowing = self.main_window.drop_down["fft_windowing"].currentText()

    # Gets called when the pcm windowing function changes
    @pyqtSlot(int)
    def pcm_windowing_changed(self, index):
        print("Val: {}".format(index))
        self.processor.pcm_windowing = self.main_window.drop_down["pcm_windowing"].currentText()

    # Gets called when QTimer expires after timing out
    @pyqtSlot()
    def timeout(self):
        self.get_free_running_clicked()
