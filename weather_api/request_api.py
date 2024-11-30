import requests
import json

class WeatherAPI:
    def __init__(self, lat, lon) -> None:
        self.lon = lon
        self.lat = lat
        self.API_key = "080ea90c1cd67058674abe5d609ede1c"
        self.result = None
        
    def initialize(self):
        try:
            response = requests.get(f"https://api.openweathermap.org/data/2.5/weather?lat={self.lat}&lon={self.lon}&appid={self.API_key}")
            # response.raise_for_status()  # Raise an exception if the status code is not 200
        except requests.RequestException as e:
            print(f"Error: {e}")
            self.result = False
        else:
            self.result = json.loads(response.content.decode())
            # print(self.result)
    
    def get_weather(self):
        if self.result:
            return self.result["weather"][0]
        else:
            raise Exception("please execute the self.initialize()")
    def get_temp(self):
        if self.result:
            return round(self.result["main"]["temp"] - 273, 1)
        else:
            raise Exception("please execute the self.initialize()")
        
    def get_humidity(self):
        if self.result:
            return self.result["main"]["humidity"]
        else:
            raise Exception("please execute the self.initialize()")       
            
    
if __name__ == "__main__":
    coor = (5.9, 100.29)
    api = WeatherAPI(coor[0], coor[1])
    print(api)
    api.initialize()
    print(api.get_temp())   
    print(api.get_humidity())
    print(api.get_weather())
    