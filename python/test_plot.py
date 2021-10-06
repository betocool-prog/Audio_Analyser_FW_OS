import pyqtgraph as pg
import numpy as np
from PyQt5 import QtWidgets
import sys
import struct

if __name__ == "__main__":

    # Create the main application instance
    app = QtWidgets.QApplication([])

    filename = '/home/betocool/testfile.dat'
    data_file = open(filename, 'rb')

    loop = True
    i = 0

    left = []
    right = []

    while loop:

        data = struct.unpack("<2i", data_file.read(8))
        left.append(data[0])
        right.append(data[1])
        i += 1
        if i == 96000:
            loop = False

    left = np.array(left)
    right = np.array(right)

    left = np.int32(left << 8)
    left = left.astype(float) / 2 ** 31 / 0.64

    right = np.int32(right << 8)
    right = right.astype(float) / 2 ** 31 / 0.64

    pg.plot(right, pen='r')
    pg.plot(left, pen='b')
    # Gracefully exit the application
    sys.exit(app.exec_())