import numpy as np
from PIL import ImageGrab, ImageTk
import cv2
import time
import tkinter as tk


RECORDING = False
EXIT = False
screen = ImageGrab.grab()
width, height = screen.size
fourcc = cv2.VideoWriter_fourcc(*'DIVX')
file_name = time.strftime('%Y_%m_%d_%H_%M_%S ', time.localtime(time.time()))
out = cv2.VideoWriter('output'+file_name+'.avi', 
                      fourcc, 10.0, (width, height))


def record():
    global RECORDING
    if not RECORDING:
        RECORDING = True
    if RECORDING:    
        b['bg'] = 'red'
    else:
        b['bg'] = 'white'    
  

def finish():
    global RECORDING
    global EXIT
    RECORDING = False
    EXIT = True
    b['bg'] = 'white'    
    out.release()


window = tk.Tk()
window.title('Recorder')
window.geometry('700x550')
frame_top = tk.Frame(window)
frame_top.pack()
frame_bottom = tk.Frame(window)
frame_bottom.pack()
canvas = tk.Canvas(frame_top, bg='green', height=480, width=700)
canvas.grid(row=0)
b = tk.Button(frame_bottom, text='record', command=record)
b.grid(row=0, column=0, sticky=tk.W)
e = tk.Button(frame_bottom, text='finish', command=finish)
e.grid(row=0, column=1)

while True:
    screen = ImageGrab.grab()
    screen_copy = screen.resize((700, 480))
    image_file = ImageTk.PhotoImage(screen_copy) 
    image = canvas.create_image(0, 0, anchor='nw', image=image_file)
    canvas.update()

    if RECORDING:
        screen_np = np.array(ImageGrab.grab())
        screen_np = cv2.cvtColor(screen_np, cv2.COLOR_RGB2BGR)
        
        out.write(screen_np)

window.mainloop() 
