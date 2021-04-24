import sys
import pyqtgraph as pg
from PyQt5 import QtWidgets


if __name__ == "__main__":

   app = QtWidgets.QApplication(sys.argv)

   # Create the main window that holds everything
   main_window = QtWidgets.QMainWindow()

   # Central widget to add layouts
   central_widget = QtWidgets.QWidget()

   # Set the central widget for the main window - Vertical layout
   main_window.setCentralWidget(central_widget)
   main_window.centralWidget().setLayout(QtWidgets.QVBoxLayout())

   #Make the layout available to add widgets
   main_layout = main_window.centralWidget().layout()

   ##############################
   # Graph widget - top element
   pg.setConfigOption('background', 'w')
   graphWidget = pg.PlotWidget()
   graphWidget.setXRange(0, 10, 0.1)
   graphWidget.setYRange(0, 12, 0.1)
   graphWidget.plot(x=[1, 2, 3, 4, 5, 6], y=[6, 5, 4, 3, 2, 1], pen='b')

   ##############################
   # Horizontal layout - second element
   button_layout = QtWidgets.QHBoxLayout()
   # Add three internal vertical layouts
   for idx in range(0, 3):
      button_layout.addLayout(QtWidgets.QVBoxLayout())

   # Add three pushbuttons on the leftmost position
   for idx in range(0, 3):
      button_layout.itemAt(0).layout().addWidget(QtWidgets.QPushButton("Left {}".format(idx + 1)))

   # A spacer item pushes items apart, in this case, it keeps the buttons aligned to the top -> Policy: expanding
   # Spacer items are not visible. QSpacerItem(width, height, horizontal policy, vertical policy)
   # The current spacer item expands to the top, as it's the last element on the bottom
   button_layout.itemAt(0).layout().addSpacerItem(QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding))

   # Add two pushbuttons in the middle
   for idx in range(0, 2):
      button_layout.itemAt(1).layout().addWidget(QtWidgets.QPushButton("Middle {}".format(idx + 1)))

   button_layout.itemAt(1).layout().addSpacerItem(QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding))

   # Add one pushbutton on the right
   button_layout.itemAt(2).layout().addWidget(QtWidgets.QPushButton("Right 1"))
   button_layout.itemAt(2).layout().addSpacerItem(QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding))

   ##############################
   # Grid layout : third element
   check_box_grid = QtWidgets.QGridLayout()

   # Add checkboxes in a grid
   for i in range(1, 4):
      for j in range(1, 5):
         check_box_grid.addWidget(QtWidgets.QCheckBox("Checkbox {}{}".format(i,j)), i, j)

   # Add graph widget first, that's the top element
   main_layout.addWidget(graphWidget)

   # Add the buttons layout with spacer, it's the second element
   main_layout.addLayout(button_layout)

   # Add the grid layout last
   main_layout.addLayout(check_box_grid)

   # Optional - Comment in or out
   # Add a spacer at the bottom to push all elements up
   main_layout.addSpacerItem(QtWidgets.QSpacerItem(2, 4, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding))

   # Resize and rename window
   main_window.resize(640, 480)
   main_window.setWindowTitle("Test Window")

   main_window.show()
   app.exec_()