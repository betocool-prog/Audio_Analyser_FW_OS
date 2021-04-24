import socket
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

class UDP_Receiver:

    def __init__(self):
        self.LOCAL_IP = "192.168.1.20"
        self.LOCAL_PORT = 8888
        self.sample_type = np.dtype([('left', np.int32), ('right', np.int32)])

    def listen(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.LOCAL_IP, self.LOCAL_PORT))
        total_len = 0;
        total_data = bytearray()
        plt.subplot(2, 1, 1)
        plt.subplot(2, 1, 2)

        while(True):

            print("Waiting for message...")
            data, addr = self.sock.recvfrom(8192)
            total_data.extend(data)
            total_len += len(data)
            print("Received: {}".format(total_len))
            total_data_bytes = bytes(total_data)

            if total_len == 8704:
                total_len = 0
                total_data = bytearray()
                print("Data type: {}".format(type(data)))
                print("Total_data type: {}".format(type(total_data_bytes)))
                samples = np.frombuffer(total_data_bytes,dtype=self.sample_type)
                left_samples = (np.int32(samples[:]['left'] << 8))
                right_samples = (np.int32(samples[:]['right'] << 8))
                plt.subplot(2, 1, 1)
                plt.cla()
                plt.plot(left_samples)
                plt.subplot(2, 1, 2)
                plt.cla()
                plt.plot(right_samples)
                plt.show()



if __name__ == "__main__":
    udp_receiver = UDP_Receiver()

    udp_receiver.listen()