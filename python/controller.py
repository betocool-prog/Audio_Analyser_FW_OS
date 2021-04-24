#!/usr/bin/env python3

from PyQt5.QtCore import pyqtSlot, QTimer, QObject

import client
import udp_receiver_thread as udp_r_t
import tcp_client_thread as tcp_c_t
import data_proc
import audio_plot_window as apw


class Controller(QObject):

    def __init__(self):

        super().__init__()

        self.getting_data = False

        self.timer = QTimer()

        self.processor = data_proc.DataProc()
        self.main_window = apw.AudioPlotMainWindow()
        self.main_window.set_fft_graph(self.processor.grFFT)
        self.main_window.set_pcm_graph(self.processor.grPCM)
        self.main_window.build_window()

        self.udp_receiver = udp_r_t.UDPReceiverThread()
        self.udp_receiver.set_nr_of_bytes_expected(36864)

        self.tcp_client = tcp_c_t.TCPClientThread()

        self.processor.set_data_source(self.udp_receiver)

        self.udp_receiver.start()
        self.udp_receiver.rx_complete_signal.connect(self.data_rdy_slot)

        # Connect signals and slots
        # Start / stop getting data
        self.timer.timeout.connect(self.timeout)
        self.main_window.buttons["get_data"].clicked.connect(self.get_adc_data_clicked)
        self.main_window.buttons["single_shot"].clicked.connect(self.get_single_shot_clicked)
        # self.main_window.checkboxes["avg"].clicked.connect(self.avg_clicked)
        # self.main_window.checkboxes["x_log"].clicked.connect(self.xlog_clicked)
        # self.main_window.checkboxes["show_fft_left"].clicked.connect(self.fft_show_clicked)
        # self.main_window.checkboxes["show_fft_right"].clicked.connect(self.fft_show_clicked)
        #
        # self.main_window.fft_rb_group.buttonClicked.connect(self.function_clicked)
        # self.main_window.pcm_rb_group.buttonClicked.connect(self.l_r_clicked)
        #
        # self.main_window.line_edit["Frequency"].returnPressed.connect(self.frequency_changed)
        # self.main_window.line_edit["Amplitude"].returnPressed.connect(self.amplitude_changed)
        # self.main_window.line_edit["Impedance"].returnPressed.connect(self.impedance_changed)
        #
        # self.main_window.drop_down["fft_windowing"].activated.connect(self.fft_windowing_changed)
        # self.main_window.drop_down["pcm_windowing"].activated.connect(self.pcm_windowing_changed)
        #
        # self.main_window.drop_down["pcm_windowing"].setCurrentIndex(0)
        # self.main_window.drop_down["fft_windowing"].setCurrentIndex(3)
        #
        # # Set default client settings
        # client.Client().set_function_type("SINE")
        # client.Client().set_frequency("1000")
        # client.Client().set_amplitude_v("1.5")

    # Gets called whenever the UDP data has arrived
    @pyqtSlot()
    def data_rdy_slot(self):
        self.processor.data_rdy_slot()

        if self.getting_data:
            self.timer.setInterval(10)
            self.timer.setSingleShot(True)
            self.timer.start()

    # Gets called when the get data button is pressed
    @pyqtSlot()
    def get_adc_data_clicked(self):
        if self.getting_data:
            self.getting_data = False
            self.main_window.buttons["get_data"].setText("Get ADC Data")

        else:
            self.main_window.buttons["get_data"].setText("Stop ADC Data")
            self.getting_data = True
            client.Client().get_adc_data()

    @pyqtSlot()
    def get_single_shot_clicked(self):
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

        if bt_name == "Impedance":
            res_value = self.main_window.line_edit[bt_name].text()
            if res_value.isdecimal():
                client.Client().set_function_type("IMPULSE")
                self.processor.impedance_calc = True
                self.processor.resistor = float(res_value)
                print(self.processor.resistor)
        else:
            self.processor.impedance_calc = False
            client.Client().set_function_type(bt_name.upper())

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
        client.Client().set_frequency(freq_val)

    # Gets called when the amplitude changes
    @pyqtSlot()
    def amplitude_changed(self):
        amp_val = self.main_window.line_edit["Amplitude"].text()
        try:
            client.Client().set_amplitude_v(amp_val)
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
        client.Client().get_adc_data()
