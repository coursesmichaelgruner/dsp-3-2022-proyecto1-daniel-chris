import numpy as np
import matplotlib.pyplot as plt
import sounddevice as sd
import argparse
import getch

parser = argparse.ArgumentParser()
parser.add_argument('duration', help = "tone duration in ms",default = 300)
parser.add_argument("-w",'--window', help = "tone window function",default = "")
args=parser.parse_args()

duration = int(args.duration)/1000
window = args.window

keyboard = {'1':[697,1209],
            '2':[697,1336],
            '3':[697,1477],
            'A':[697,1633],
            '4':[770,1209],
            '5':[770,1336],
            '6':[770,1477],
            'B':[770,1633],
            '7':[852,1209],
            '8':[852,1336],
            '9':[852,1477],
            'C':[852,1633],
            '*':[941,1209],
            '0':[941,1336],
            '#':[941,1477],
            'D':[941,1633]}
Fs = 8000.0

n = np.arange(duration*Fs)

match window:
    case "hann":
        win = (1-np.cos(2*np.pi*n/(duration*Fs)))/2
    case "triag":
        win = 1-abs((n-duration*Fs/2)/((Fs*duration+1)/2))
    case _:
        win = 1

while(True):
    key = getch.getch()
    try:
        ftones = keyboard.get(key)
        print(ftones)
#Note function

        y = (np.sin(2*np.pi*ftones[0]*n/Fs)/2+np.sin(2*np.pi*ftones[1]*n/Fs)/2)*win
#playback
        sd.play(y,Fs)
        sd.wait()
    except:
        pass
sd.stop()
