"""EE 250L Lab 11 Final Project

web_server.py: Event logging server.

By- Krtin Jain and Vedant Nevatia

Repository Link- https://github.com/usc-ee250-fall2019/finalproj-kj_ved.git

"""
from flask import jsonify
from flask import request
from flask import Flask
import argparse
import json
from datetime import datetime

app = Flask('Light Sensor')   #flask app named light sensor

array = []                    #create array
@app.route('/history')        #store history of logs into array   
def logs():
	log = ""
	for i in array:
		log += i + "<br>"
	return log

@app.route('/past', methods=['POST'])              
def adc_callback():                        #create a function to store the time of created logs 
	n = datetime.now()
	time = n.strftime("%H:%M:%S: ")
	msg = request.get_json()
	p = msg['message']                    # add the adc values and time of reception in the array
	array.append(time + p)

	return "\n"

if __name__ == '__main__':
	n = datetime.now()
	time = n.strftime("%H:%M:%S: ")
	app.run(debug=False, host='0.0.0.0', port=5000)