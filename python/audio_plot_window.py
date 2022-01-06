#!/usr/bin/env python3

from PyQt5 import QtWidgets


class AudioPlotMainWindow(QtWidgets.QMainWindow):

    # AudioPlotMain is a subclass of QMainWindow
    # It adds the signals and slots for
    # data showing and processing

    def __init__(self):

        super().__init__()

        # Central widget to add layouts
        # Set the central widget for the main window - Vertical layout
        super().setCentralWidget(QtWidgets.QWidget())
        super().centralWidget().setLayout(QtWidgets.QVBoxLayout())

        # Make the layout available to add widgets
        self.main_layout = super().centralWidget().layout()

        self.checkboxes = {}
        self.fft_layout = QtWidgets.QGridLayout()
        self.pcm_layout = QtWidgets.QGridLayout()

        self.buttons = {}
        self.pcm_rb = {}
        self.pcm_rb_group = QtWidgets.QButtonGroup()
        self.fft_rb = {}
        self.fft_rb_group = QtWidgets.QButtonGroup()
        self.line_edit = {}
        self.text_edit = {}
        self.text_labels = {}
        self.drop_down = {}

        self.fft_graph = None
        self.pcm_graph = None

    def set_fft_graph(self, fft_graph):
        self.fft_graph = fft_graph

    def set_pcm_graph(self, pcm_graph):
        self.pcm_graph = pcm_graph

    def build_window(self):

        # Add FFT graph
        # Grid layout under FFT
        # PCM graph
        # Grid layout under PCM
        self.main_layout.addWidget(self.fft_graph)
        self.main_layout.addLayout(self.fft_layout)
        self.main_layout.addWidget(self.pcm_graph)
        self.main_layout.addLayout(self.pcm_layout)

        # FFT Grid layout
        # Checkboxes
        self.checkboxes["avg"] = QtWidgets.QCheckBox("Average")
        self.checkboxes["avg"].setChecked(False)
        self.checkboxes["show_fft_left"] = QtWidgets.QCheckBox("Show Left")
        self.checkboxes["show_fft_left"].setChecked(True)
        self.checkboxes["avg"].setChecked(False)
        layout = QtWidgets.QHBoxLayout()
        layout.addWidget(self.checkboxes["avg"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.fft_layout.addLayout(layout, 0, 0)

        layout = QtWidgets.QHBoxLayout()
        layout.addWidget(self.checkboxes["show_fft_left"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.fft_layout.addLayout(layout, 0, 3)

        self.checkboxes["x_log"] = QtWidgets.QCheckBox("Log X Axis")
        self.checkboxes["x_log"].setChecked(False)
        self.checkboxes["show_fft_right"] = QtWidgets.QCheckBox("Show Right")
        self.checkboxes["show_fft_right"].setChecked(True)
        layout = QtWidgets.QHBoxLayout()
        layout.addWidget(self.checkboxes["x_log"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.fft_layout.addLayout(layout, 1, 0)

        layout = QtWidgets.QHBoxLayout()
        layout.addWidget(self.checkboxes["show_fft_right"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.fft_layout.addLayout(layout, 1, 3)

        # Function Radio buttons
        self.fft_rb["Sine"] = QtWidgets.QRadioButton("Sine")
        self.fft_rb["Sine"].setChecked(True)
        self.fft_rb["Impulse"] = QtWidgets.QRadioButton("Impulse")
        layout = QtWidgets.QHBoxLayout()
        for item in self.fft_rb.items():
            value = item[1]
            layout.addWidget(value)
            self.fft_rb_group.addButton(value)
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.fft_layout.addLayout(layout, 0, 1)

        # Impedance line edit
        layout = QtWidgets.QHBoxLayout()
        self.line_edit["Impedance"] = QtWidgets.QLineEdit("1000")
        self.line_edit["Impedance"].setMaxLength(12)
        self.line_edit["Impedance"].setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Ignored)
        self.fft_rb["Impedance"] = QtWidgets.QRadioButton("Impedance")
        layout.addWidget(self.fft_rb["Impedance"])
        self.fft_rb_group.addButton(self.fft_rb["Impedance"])
        layout.addWidget(self.line_edit["Impedance"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.fft_layout.addLayout(layout, 0, 2)

        # FFT window list
        layout = QtWidgets.QHBoxLayout()
        self.text_labels["fft_windowing"] = QtWidgets.QLabel("Windowing Function:")
        self.drop_down["fft_windowing"] = QtWidgets.QComboBox()
        self.drop_down["fft_windowing"].insertItem(0, "Square")
        self.drop_down["fft_windowing"].insertItem(1, "Hamming")
        self.drop_down["fft_windowing"].insertItem(2, "Hanning")
        self.drop_down["fft_windowing"].insertItem(3, "Blackman")
        layout.addWidget(self.text_labels["fft_windowing"])
        layout.addWidget(self.drop_down["fft_windowing"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.fft_layout.addLayout(layout, 1, 1)

        # PCM Grid Layout
        layout = QtWidgets.QHBoxLayout()
        self.buttons["free_running"] = QtWidgets.QPushButton("Free Running")
        self.buttons["single_shot"] = QtWidgets.QPushButton("Single Shot")
        layout.addWidget(self.buttons["free_running"])
        layout.addWidget(self.buttons["single_shot"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.pcm_layout.addLayout(layout, 0, 0)

        # Create dictionary Radiobuttons L + R (t), L/R, R/L
        self.pcm_rb["L + R"] = QtWidgets.QRadioButton(r"L + R")
        self.pcm_rb["L + R"].setChecked(True)
        self.pcm_rb["L(R)"] = QtWidgets.QRadioButton(r"L(R)")
        self.pcm_rb["R(L)"] = QtWidgets.QRadioButton(r"R(L)")
        layout = QtWidgets.QHBoxLayout()
        for item in self.pcm_rb.items():
            value = item[1]
            layout.addWidget(value)
            self.pcm_rb_group.addButton(value)
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.pcm_layout.addLayout(layout, 0, 1)

        # Frequency line edit
        layout = QtWidgets.QHBoxLayout()
        self.text_labels["Frequency"] = QtWidgets.QLabel("Frequency:")
        self.line_edit["Frequency"] = QtWidgets.QLineEdit("1000")
        self.line_edit["Frequency"].setMaxLength(12)
        self.line_edit["Frequency"].setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Ignored)
        layout.addWidget(self.text_labels["Frequency"])
        layout.addWidget((self.line_edit["Frequency"]))
        layout.addSpacerItem(QtWidgets.QSpacerItem(
            2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.pcm_layout.addLayout(layout, 1, 0)

        # Amplitude line edit
        layout = QtWidgets.QHBoxLayout()
        self.text_labels["Amplitude"] = QtWidgets.QLabel("Amplitude:")
        self.line_edit["Amplitude"] = QtWidgets.QLineEdit("1.4")
        self.line_edit["Amplitude"].setMaxLength(12)
        self.line_edit["Amplitude"].setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Ignored)
        layout.addWidget(self.text_labels["Amplitude"])
        layout.addWidget((self.line_edit["Amplitude"]))
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.pcm_layout.addLayout(layout, 1, 1)

        # PCM window list
        layout = QtWidgets.QHBoxLayout()
        self.text_labels["pcm_windowing"] = QtWidgets.QLabel("Windowing Function:")
        self.drop_down["pcm_windowing"] = QtWidgets.QComboBox()
        self.drop_down["pcm_windowing"].insertItem(0, "Square")
        self.drop_down["pcm_windowing"].insertItem(1, "Hamming")
        self.drop_down["pcm_windowing"].insertItem(2, "Hanning")
        self.drop_down["pcm_windowing"].insertItem(3, "Blackman")
        layout.addWidget(self.text_labels["pcm_windowing"])
        layout.addWidget(self.drop_down["pcm_windowing"])
        layout.addSpacerItem(
            QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding))
        self.pcm_layout.addLayout(layout, 0, 2)

        # Resize and rename window
        super().resize(960, 960)
        super().setWindowTitle("Audio Analyser Window")