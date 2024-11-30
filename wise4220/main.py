import requests
import json

class WiseAPI:
    def __init__(self) -> None:
        self.url = "http://192.168.7.158:80/sensor_value/slot_0"

        self.headers = {
            "Cookie": "adamsessionid=20E59E85E04",
        }
        
    def initialize(self):
        try:
            response = requests.get(self.url, headers=self.headers)
            # response.raise_for_status()  # Raise an exception if the status code is not 200
        except requests.RequestException as e:
            print(f"Error: {e}")
            self.result = False
        else:
            self.result = json.loads(response.content.decode())
            # print(self.result)
    
    def get_temp(self):
        if self.result:
            return self.result["SVal"][0]["EgF"]
        else:
            raise Exception("please execute the self.initialize()")
        
    def get_humidity(self):
        if self.result:
            return self.result["SVal"][1]["EgF"]
        else:
            raise Exception("please execute the self.initialize()")       
            
    
if __name__ == "__main__":
    api = WiseAPI()
    # print(api)
    api.initialize()
    print(api.get_temp())   
    print(api.get_humidity())

    


