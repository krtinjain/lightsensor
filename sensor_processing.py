
#  By- Krtin Jain
 
import paho.mqtt.client as mqtt
import requests
import time
import json



def on_connect(client, userdata, flags, rc):
    print("Connected to server (i.e., broker) with result code "+str(rc)) #connecting to server using subscribe() function
    client.subscribe("rpi-krtinjain/light")
    client.message_callback_add("rpi-krtinjain/light", light_callback)                

def on_message(client, userdata, msg):
    print("on_message: " + msg.topic + " " + str(msg.payload, "utf-8"))  

sense = 0 #flags
on = 1

def light_callback(client, userdata, msg):
	adc = str(msg.payload,"utf-8") #finding value of message
    print(adc)
    t = 300 # threshold set to 300
   
    p = {'Content-Type': 'application/json', 'Authorization': None } 
    pld = {} #payload
    
    global on

    if (int(adc) < t and on == 1):             #check if threshold is higher and print LIGHTS_OFF then send the information
        pld = {  'message': 'LIGHTS_OFF' , }
        rs = requests.post("http://localhost:5000/log", headers=p, data=json.dumps(pld)) 
        print(rs)
    	on = 0

    elif (int(adc) > t and on == 0):         #check if threshold is lower and print LIGHTS_ONN then send the information
        pld = { 'message': 'LIGHTS_ON', }
        rs = requests.post("http://localhost:5000/log", headers=p, data=json.dumps(pld))
        print(rs)
    	on = 1

if __name__ == '__main__':                                   
    client = mqtt.Client()
    client.on_message = on_message
    client.on_connect = on_connect
    client.connect(host="eclipse.usc.edu", port=11000, keepalive=60)
    client.loop_start()
    
    while True:
        time.sleep(1)