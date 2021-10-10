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

    plot = pg.plot()
    plot.setAspectLocked()

    # Add polar grid lines
    plot.addLine(x=0, pen=0.2)
    plot.addLine(y=0, pen=0.2)
    for r in range(2, 20, 2):
        circle = pg.QtGui.QGraphicsEllipseItem(-r, -r, r * 2, r * 2)
        circle.setStartAngle(210 * 16)
        circle.setSpanAngle(120 * 16)
        circle.setPen(pg.mkPen(0.2))
        plot.addItem(circle)

    # make polar data
    import numpy as np

    theta = np.linspace(0, 2 * np.pi, 100)
    radius = np.random.normal(loc=10, size=100)

    # Transform to cartesian and plot
    x = radius * np.cos(theta)
    y = radius * np.sin(theta)
    plot.plot(x, y)

    # Gracefully exit the application
    sys.exit(app.exec_())