import requests

r = requests.get('https://api-dccs-ensaas.education.wise-paas.com/api/v1/Projects')

print(r.text)