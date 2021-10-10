import socket
import analyser_pb2
import cmd, sys

class CLI(cmd.Cmd):

    intro = " Command line interpreter for Audio Analyser "
    prompt = 'pb-shell>>>'

    def __init__(self):
        self.client = Client()
        super().__init__()

    def do_get_adc_data(self, arg):
        self.client.get_adc_data()

    def do_reset(self, arg):
        self.client.reset(arg)

    def do_EOF(self, arg):
        print("")
        return True

    def do_exit(self, arg):
        print("")
        return True

    def do_test(self, arg):
        print("Client Test {}".format(arg))
        self.client.test(arg)

    def do_set_frequency(self, arg):
        self.client.set_frequency(arg)

    def do_set_amplitude(self, arg):
        self.client.set_amplitude(arg)

    def do_set_mode(self, arg):
        self.client.set_mode(arg)

    def do_set_function_type(self, arg):
        self.client.set_function_type(arg)


class Client:

    xfer_id = 0

    def __init__(self):
        self.rpc = RPC()

    def get_adc_data(self):
        msg = analyser_pb2.Service()
        msg.get_adc_data = True
        msg.message_type = analyser_pb2.ACTION_MESSAGE
        # RPC().send_pb_message(msg)
        reply = self.rpc.send_pb_message_and_wait(msg)

        if reply is not None:
            rsp = analyser_pb2.Service()
            rsp.ParseFromString(reply)

    def get_delay(self):
        value = 0
        msg = analyser_pb2.Service()
        msg.delay = 0
        msg.message_type = analyser_pb2.GET_MESSAGE;
        reply = self.rpc.send_pb_message_and_wait(msg)

        if reply is not None:
            rsp = analyser_pb2.Service()
            rsp.ParseFromString(reply)
            value = rsp.delay

        return value

    def reset(self, arg):
        self.xfer_id += 1
        msg = analyser_pb2.Service()
        msg.command.reset = True
        msg.message_type = analyser_pb2.ACTION_MESSAGE;
        msg.xfer_id = self.xfer_id
        self.rpc.send_pb_message(msg)

    def test(self, arg):
        print("In Class Commands: {}".format((arg)))

    def set_frequency(self, arg):

        if arg is None:
            print("Expected argument in the form set_frequency value")
        else:
            try:
                frequency = int(arg)
                if frequency >= 48000:
                    raise ValueError
                msg = analyser_pb2.Service()
                msg.message_type = analyser_pb2.SET_MESSAGE
                msg.signalconfig.frequency = frequency
                self.rpc.send_pb_message(msg)
            except Exception as e:
                print("Exception: {}".format(repr(e)))

    def set_amplitude(self, arg):

        if arg is None:
            print("Expected argument in the form set_amplitude value")
        else:
            try:
                float(arg)
                amplitude = float(arg) / 1.6
                if (amplitude > 1) or (amplitude < -1):
                    amplitude = 1
                msg = analyser_pb2.Service()
                msg.message_type = analyser_pb2.SET_MESSAGE
                msg.signalconfig.amplitude = abs(amplitude)
                self.rpc.send_pb_message(msg)
            except Exception as e:
                print("Exception: {}".format(repr(e)))

    def set_amplitude_v(self, arg):

        if arg is None:
            print("Expected argument in the form set_amplitude_v value")
        else:
            try:
                amplitude = float(arg) / 1.75
                if (amplitude > 1) or (amplitude < -1):
                    amplitude = 1
                msg = analyser_pb2.Service()
                msg.message_type = analyser_pb2.SET_MESSAGE
                msg.pulsed.amplitude = abs(amplitude)
                self.rpc.send_pb_message(msg)
            except Exception as e:
                print("Exception: {}".format(repr(e)))

    def set_mode(self, arg):

        value = 0

        if arg is None:
            print("Expected argument in the form set_mode MODE")

        elif arg.isnumeric():

            if (int(arg) == analyser_pb2.PULSED) or (int(arg) == analyser_pb2.CONTINUOUS):
                value = int(arg)

            else:
                print("Expected Values {} or {}".format(analyser_pb2.PULSED, analyser_pb2.CONTINUOUS))

        else:
            try:
                value = getattr(analyser_pb2, arg)
                msg = analyser_pb2.Service()
                msg.message_type = analyser_pb2.SET_MESSAGE
                msg.mode = value
                self.rpc.send_pb_message(msg)

            except Exception as e:
                print(repr(e))

    def set_function_type(self, arg):

        value = 0

        if arg is None:
            print("Expected argument in the form set_function_type FUNCTION_TYPE")

        elif arg.isnumeric():

            if (int(arg) == analyser_pb2.SINE) or (int(arg) == analyser_pb2.IMPULSE):
                value = int(arg)

            else:
                print("Expected Values {} or {}".format(analyser_pb2.SINE, analyser_pb2.IMPULSE))

        else:
            try:
                value = getattr(analyser_pb2, arg)
                msg = analyser_pb2.Service()
                msg.message_type = analyser_pb2.SET_MESSAGE
                msg.pulsed.function_type = value
                self.rpc.send_pb_message(msg)

            except Exception as e:
                print(repr(e))


class RPC:

    def __init__(self):
        self.UDP_IP = "192.168.1.144"
        self.UDP_PORT = 5003
        self.xfer_id = 0

    def send_pb_message(self, msg):

        if isinstance(msg, analyser_pb2.Service):

            self.xfer_id += 1;
            msg.xfer_id = self.xfer_id
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.sendto(msg.SerializeToString(), (self.UDP_IP, self.UDP_PORT))
            sock.close()
        else:

            print("Cant send message as it's type {}".format(type(msg)))

    def send_pb_message_and_wait(self, msg):
        reply = None

        if isinstance(msg, analyser_pb2.Service):

            self.xfer_id += 1;
            msg.xfer_id = self.xfer_id
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.settimeout(0.25)

            try:
                sock.sendto(msg.SerializeToString(), (self.UDP_IP, self.UDP_PORT))
                reply = sock.recv(1500)
                sock.close()

            except socket.error:
                print("Socket send and receive failed: {}".format(socket.error))
        else:
            print("Cant send message as it's type {}".format(type(msg)))

        return reply