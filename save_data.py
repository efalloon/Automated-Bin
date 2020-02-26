#Created By Euan Traynor, 2020
import serial
import json

#variables
counter_objects = 0
start = ""
end = ""
times_used = 0
ultra_1_d = ""
ultra_2_d = ""
final_d = ""
bin_full = "false"
bin_empty = "false"
date = ""

bin_has_been_emptied = "false"
previous_bin_is_full = "false"

#this function allows the data gathered to be written to the json file for storage.
def writeToJSONFile(path, fileName, data):
    filePathNameWExt = './' + path + '/' + fileName + '.json'
    with open(filePathNameWExt, 'w') as fp:
        json.dump(data, fp, indent=2)

#this sets up the arduino port and serial
arduinoData = serial.Serial("/dev/cu.wchusbserial14330", 9600) #port name may change depending on laptop OS
while True:
    #get the output from arduino
    myData = (arduinoData.readline().strip()).decode("utf-8")
    print(myData)

    #if the line contains anyone of the following, it will retrieve the most important information from that line
    if "motion detected at" in myData:
        myData = myData.replace('motion detected at ', '')
        start = myData
        counter_objects += 1
    elif "Times bin has been used" in myData:
        myData = myData.replace('Times bin has been used: ', '')
        times_used = int(myData)
        counter_objects += 1
    elif "Ultrasonic 1 Distance" in myData:
        myData = myData.replace('Ultrasonic 1 Distance = ', '')
        ultra_1_d = myData
        counter_objects += 1
    elif "Ultrasonic 2 Distance" in myData:
        myData = myData.replace('Ultrasonic 2 Distance = ', '')
        ultra_2_d = myData
        counter_objects += 1
    elif "Final Approximate Distance" in myData:
        myData = myData.replace('Final Approximate Distance = ', '')
        final_d = myData
        counter_objects += 1
    elif "Bin is full" in myData:
        myData = myData.replace('Bin is full: ', '')
        bin_full = myData
        counter_objects += 1
    elif "Bin is empty" in myData:
        myData = myData.replace('Bin is empty: ', '')
        bin_empty = myData
        counter_objects += 1
    elif "Date" in myData:
        myData = myData.replace('Date: ', '')
        date = myData
        counter_objects += 1
    elif "motion ended at" in myData:
        myData = myData.replace('motion ended at ', '')
        end = myData
        counter_objects += 1
    
    #when all the information has been gathered, the following will run
    if counter_objects == 9:

        if previous_bin_is_full == "true" and bin_empty == "true":
            bin_has_been_emptied = "true"

        if bin_full == "true":
            previous_bin_is_full = "true"
        else:
            previous_bin_is_full = "false"
        
        data = []

        #get currently existing json, so that it may be restored once new data is saved
        with open('index.json') as json_file:
            #make sure json file (if empty) is set to []
            get = json.load(json_file)
            for item in get:
                data.append(item)

        #setting up new json dictionary and tuples
        item = {}
        item['start'] = start
        item['end'] = end
        item['Times Used'] = times_used
        item['Ultrasonic sensor (1) Reading'] = ultra_1_d
        item['Ultrasonic sensor (2) Reading'] = ultra_2_d
        item['Final Level of Trash'] = final_d
        item['Bin has been emptied'] = bin_has_been_emptied
        item['date'] = date

        data.append(item)

        #save 'data' list to JSON file
        writeToJSONFile('./','index',data)

        #reset
        counter_objects = 0
        start = ""
        end = ""
        times_used = ""
        ulta_1_d = ""
        ulta_2_d = ""
        final_d = ""
        bin_full = ""
        bin_empty = ""
        date = ""
        bin_has_been_emptied = "false"

    
    
