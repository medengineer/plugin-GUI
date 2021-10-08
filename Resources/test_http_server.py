import os
import platform
import subprocess
import signal

import requests
import json
import time 


system = platform.system()

address = 'http://127.0.0.1:37497'

if system == 'Linux':
	gui_dir= '/home/pavel/Projects/Allen/OpenEphys/plugin-GUI/Build/Debug/open-ephys'
	recording_dir= '/home/pavel/open-ephys'
elif system == 'Windows':
	pass #TODO
elif system == 'Darwin': #?
	gui_dir= '/Users/runner/work/plugin-GUI/plugin-GUI/Build/Release/open-ephys.app/Contents/MacOS/open-ephys /Users/runner/work/plugin-GUI/plugin-GUI/Resources/testConfig.xml'
	recording_dir= '/Users/runner/work/plugin-GUI/plugin-GUI'

try:

	#Launch open-ephys
	p = subprocess.Popen(gui_dir, stdout=subprocess.PIPE, shell=True, preexec_fn=os.setsid)

	#Wait for the GUI to launch
	time.sleep(5)

	#Connet to open-ephys
	r = requests.get(address)
	print("Connected to OpenEphys instance on " + address)

	#Get the list of processors
	#resp = requests.get(address + '/api/processors')
	#data = resp.json()
	#processors = data['processors']
	#print(processors[0]['id'])

	#Configure the first processor (FileReader for now)
	#payload = { 'loadFile' : '<path-to-scrubber-test-file>' }
	#url = '/api/processors/' + str(processors[0]['id']) + '/config'
	#requests.put(address + url, data = json.dumps(payload))

	time.sleep(1)

	for i in range(1,3):

		#Start acquisition
		payload = { 
			'mode' : 'ACQUIRE',
			'data_parent_dir' : recording_dir }
		url = '/api/status'
		requests.put(address + url, data = json.dumps(payload))

		#Wait 2 seconds
		time.sleep(2)

		#Start recording
		payload = { 
			'mode' : 'RECORD',
			'data_parent_dir' : recording_dir }
		url = '/api/status'
		requests.put(address + url, data = json.dumps(payload))

		#Wait 2 seconds
		time.sleep(2)

		#Stop recording
		payload = { 
			'mode' : 'IDLE',
			'data_parent_dir' : recording_dir }
		url = '/api/status'
		requests.put(address + url, data = json.dumps(payload))

		#Wait 2 seconds (for visual confirmation recording has stopped)
		time.sleep(2)

	os.killpg(os.getpgid(p.pid), signal.SIGTERM)

	## Check output files



except requests.exceptions.Timeout:
    # Maybe set up for a retry, or continue in a retry loop
    print("Timeout")
except requests.exceptions.TooManyRedirects:
    # Tell the user their URL was bad and try a different one
    print("Bad URL")
except requests.exceptions.RequestException as e:
    # catastrophic error. bail.
    print("Very bad")
    raise SystemExit(e)