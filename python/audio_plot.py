#!/usr/bin/env python

import sys

from PyQt5 import QtWidgets
import controller as ctrl

if __name__ == "__main__":

    # Create the main application instance
    app = QtWidgets.QApplication([])

    controller = ctrl.Controller()
    controller.main_window.show()

    # Gracefully exit the application
    sys.exit(app.exec_())
