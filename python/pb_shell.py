#!/usr/bin/env python3.7
import client
from PyQt5.QtCore import QThread, pyqtSignal


class PB_Shell(QThread):

    def __init__(self):
        QThread.__init__(self)
        self.client = client.CLI()

    def run(self):
        self.client.cmdloop()

    def stop(self):
        self.client.do_exit(0)


if __name__ == "__main__":

    pb_shell = PB_Shell()
    pb_shell.client.cmdloop()
