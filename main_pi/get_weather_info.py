import urllib.request
import json

def request(url):
    response = urllib.request.urlopen(url)
    byte_data = response.read()
    json_data = byte_data.decode('utf-8')
    return json_data

def get_weather():
    weather_url = 'http://api.openweathermap.org/data/2.5/weather?id=1835553&appid=ec235bcd4a9a4eb4b28fd7ed1bac74e8'
    weather_json = json.loads(request(weather_url))
#    print(weather_json)
    return weather_json['weather'][0]['id']

def get_pms():
    pm_url = 'https://api.waqi.info/feed/suwon/?token=ea253fbb77ecc4c8fc82dc4487a32a9211e3d12b'
    return json.loads(request(pm_url))

def get_pm10():
    pm_json = get_pms()
    return pm_json['data']['iaqi']['pm10']['v']
    
def get_pm25():
    pm_json = get_pms()
    return pm_json['data']['iaqi']['pm25']['v']
