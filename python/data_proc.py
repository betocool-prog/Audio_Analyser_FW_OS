#!/usr/bin/env python3

import numpy as np
from PyQt5.QtCore import pyqtSlot
import pyqtgraph as pg

import client


class DataProc():

    def __init__(self, signal_config=None):

        self.average = False
        self.avg_nr = 64
        self.signal_config = signal_config

        self.pcm_mode = r"L + R"

        self.left_avg_fft = np.zeros([self.avg_nr, 2048], dtype=complex)
        self.right_avg_fft = np.zeros([self.avg_nr, 2048], dtype=complex)
        self.idx = 0

        # Impedance calculation
        self.resistor = 1000
        self.impedance_calc = False

        pg.setConfigOption('background', 'w')
        self.grFFT = pg.PlotWidget()
        self.grFFT.plotItem.showGrid(True, True, 0.7)
        self.grFFT.plotItem.disableAutoRange()
        self.grFFT.plotItem.setYRange(-160, 5, padding=0.02)
        self.grFFT.plotItem.setXRange(1, 48000, padding=0)

        self.grPCM = pg.PlotWidget()
        self.grPCM.plotItem.showGrid(True, True, 0.7)
        self.grPCM.plotItem.setXRange(0, 4096, padding=0)
        self.grPCM.plotItem.setYRange(-1.5, 1.5, padding=0.02)

        self.rescale = False

        self.show_fft_left = True
        self.show_fft_right = True

        self.data_source = None

        self.window_function = {
            "Square" : np.ones,
            "Blackman" : np.blackman,
            "Hamming" : np.hamming,
            "Hanning" : np.hanning
        }

        self.pcm_windowing = "Square"
        self.fft_windowing = "Blackman"

    # Gets called whenever data has arrived, from audio_plot
    @pyqtSlot()
    def data_rdy_slot(self):
        left_y = self.data_source.left_samples.astype(float) / 2 ** 31 / 0.699
        right_y = self.data_source.right_samples.astype(float) / 2 ** 31 / 0.693

        signal_config = client.Client().get_signal_config()
        delay = signal_config.delay
        # delay = 0
        delay_samples = int(delay * 96 / 10000) + 360
        # delay_position = 182 + 205 - delay_samples
        # print("Delay samples: {}".format(delay_samples))
        if self.signal_config.op_mode == getattr(client.analyser_pb2, 'SYNC'):
            start = delay_samples
            end = delay_samples + self.signal_config.fft_size # 8192 * 4
        else:
            start = 0
            end = self.signal_config.fft_size

        left_x = np.linspace(0, 4096 - 1, 4096)
        right_x = np.linspace(0, 4096 - 1, 4096)
        left_y = left_y[start:end]
        left_plot_y = left_y * self.window_function[self.pcm_windowing](len(left_y))
        right_y = right_y[start:end]
        right_plot_y = right_y * self.window_function[self.pcm_windowing](len(right_y))
        # if self.pcm_mode == r"L + R":
        self.grPCM.plot(left_plot_y, pen="b", clear=True)
        self.grPCM.plot(right_plot_y, pen="r", clear=False)
        # elif self.pcm_mode == r"L(R)":
        #     self.grPCM.plot(right_plot_y, left_plot_y, pen="b", clear=True)
        # elif self.pcm_mode == r"R(L)":
        #     self.grPCM.plot(left_plot_y, right_plot_y, pen="b", clear=True)

        if (np.any(left_y)) and (np.any(right_y)):
            windowed_left_y = left_y * self.window_function[self.fft_windowing](len(left_y))
            windowed_right_y = right_y * self.window_function[self.fft_windowing](len(right_y))
            left_fft = (2 * np.fft.fft(windowed_left_y) / len(windowed_left_y))[0:(len(windowed_left_y) / 2).__int__()]
            right_fft = (2 * np.fft.fft(windowed_right_y) / len(windowed_right_y))[0:(len(windowed_right_y) / 2).__int__()]

            # if self.average:
            #
            #     self.left_avg_fft[self.idx] = left_fft
            #     self.right_avg_fft[self.idx] = right_fft
            #
            #     self.idx += 1
            #     self.idx = self.idx % self.avg_nr
            #
            #     left_fft = np.average(self.left_avg_fft, axis=0)
            #     right_fft = np.average(self.right_avg_fft, axis=0)

            left_fft_db = 20 * np.log10(np.abs(left_fft))
            right_fft_db = 20 * np.log10(np.abs(right_fft))

            x_data = (np.linspace(0, len(left_y) - 1, len(left_y)))[1:2048]

            # if self.impedance_calc:
            #     q = np.abs(right_fft / left_fft)
            #     impedance = q / (1 - q) * self.resistor
            #     self.grFFT.plot(x_data * 48000. / 2048, impedance[1:2048], pen="b", clear=True)

            # else:
            if self.show_fft_left:
                # self.grFFT.plot(x_data * 48000. / 2048, left_fft_db[1:2048], pen="r", clear=True)
                self.grFFT.plot(left_fft_db[1:2048], pen="r", clear=True)

            if self.show_fft_right:
                # self.grFFT.plot(x_data * 48000. / 2048, right_fft_db[1:2048], pen="b", clear=not self.show_fft_left)
                self.grFFT.plot(right_fft_db[1:2048], pen="b", clear=not self.show_fft_left)

    def set_data_source(self, data_source):
        self.data_source = data_source

    def set_signal_config(self, signal_config):
        self.signal_config = signal_config
