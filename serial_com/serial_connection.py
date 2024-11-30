import serial
import time
import json

class SerialConnection:
    def __init__(self, port_no):
        self.port_no = port_no
        self.baurate = 115200
        self.sr = None
        self.connection_trials = 3
        self.no_data_bytes = None
        self.last_data = {"position":[0],"current":[0],"power":[0],"day_time":[0],"rain_detection":[0],"motion_detection":[0]}
        self.keys = [x for x in self.last_data]

    def start_connection(self):
        if self.port_no is not None:
            port = f"COM{self.port_no}"
            try:
                self.sr = serial.Serial(port, self.baurate)
            except Exception:
                if self.connection_trials > 0:
                    self.start_connection()
                    self.connection_trials -= 1
                else:
                    raise("Check the COM connection with the esp32")
        else:
            raise("Please specify the number of the COM connected.")
    
    def read_data(self):
        if self.sr is None:
            print("Serial connection not established.")
            return self.last_data
        
        try:
            self.no_data_bytes = self.sr.in_waiting
        except AttributeError:
            print("No incoming data")
            return self.last_data
            
        else:
            if self.no_data_bytes > 0 and not None:
                recv_data = self.sr.read(self.no_data_bytes).decode().strip("\n")
                # print(recv_data)
                if recv_data[:1] == "{" and recv_data[-2:-1] == "}":
                    try:
                        recv_data = json.loads(recv_data)
                    except Exception as e:
                        print(e)
                    except json.JSONDecodeError:
                        print("Failed to decode the string to dict.")
                    else:
                        if isinstance(recv_data, dict) and [x for x in recv_data] == self.keys:
                            if recv_data["position"] == "":
                                return self.last_data
                            self.last_data = recv_data
                            recv_data = {key: value.split() for key, value in recv_data.items()}
                            return recv_data
            return self.last_data

if __name__ == "__main__":
    sr = SerialConnection(port_no=8)
    sr.start_connection()
    while True:
        data = sr.read_data()
        print(data)
        time.sleep(0.2)