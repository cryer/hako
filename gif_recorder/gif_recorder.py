import tkinter as tk
from tkinter import filedialog, messagebox
from PIL import ImageGrab, Image
import threading
import time
import win32gui
import win32ui
import ctypes
from ctypes import wintypes

try:
    ctypes.windll.user32.SetProcessDPIAware()
except AttributeError:
    pass


class GifRecorder(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("GIF 录制工具")
        self.geometry("350x250")
        self.recording = False
        self.frames = []
        self.record_mode = tk.StringVar(value="area")
        self.selected_window_handle = None

        self.create_widgets()

    def create_widgets(self):
        tk.Label(self, text="录制模式:").pack(pady=5)
        tk.Radiobutton(self, text="自定义区域", variable=self.record_mode, value="area").pack(anchor=tk.W)
        tk.Radiobutton(self, text="选择窗口", variable=self.record_mode, value="window").pack(anchor=tk.W)

        tk.Label(self, text="帧间隔 (ms):").pack(pady=5)
        self.interval_entry = tk.Entry(self, width=10)
        self.interval_entry.insert(0, "100")
        self.interval_entry.pack()

        self.start_button = tk.Button(self, text="开始录制", command=self.start_recording)
        self.start_button.pack(pady=10)

        self.stop_button = tk.Button(self, text="结束录制", command=self.stop_recording, state=tk.DISABLED)
        self.stop_button.pack()

    def start_recording(self):
        self.recording = True
        self.start_button.config(state=tk.DISABLED)
        self.stop_button.config(state=tk.NORMAL)
        self.frames = []

        if self.record_mode.get() == "area":
            self.withdraw()
            self.area_selector = AreaSelector(self)
        else:
            self.withdraw()
            self.window_selector = WindowSelector(self)

    def stop_recording(self):
        self.recording = False
        self.start_button.config(state=tk.NORMAL)
        self.stop_button.config(state=tk.DISABLED)
        self.deiconify()

        if self.frames:
            self.save_gif()

    def capture_frames(self, bbox=None):
        user32 = ctypes.windll.user32
        user32.PrintWindow.argtypes = [wintypes.HWND, wintypes.HDC, wintypes.UINT]
        user32.PrintWindow.restype = wintypes.BOOL

        interval_ms = int(self.interval_entry.get()) / 1000.0
        while self.recording:
            try:
                if self.record_mode.get() == "window" and self.selected_window_handle:
                    if not win32gui.IsWindow(self.selected_window_handle):
                        print("Window not found, stopping.")
                        self.stop_recording()
                        break
                        
                    left, top, right, bot = win32gui.GetClientRect(self.selected_window_handle)
                    w = right - left
                    h = bot - top

                    if w <= 0 or h <= 0:
                        time.sleep(interval_ms)
                        continue

                    hwndDC = win32gui.GetWindowDC(self.selected_window_handle)
 
                    mfcDC = win32ui.CreateDCFromHandle(hwndDC)

                    saveDC = mfcDC.CreateCompatibleDC()

                    saveBitMap = win32ui.CreateBitmap()
                    saveBitMap.CreateCompatibleBitmap(mfcDC, w, h)

                    saveDC.SelectObject(saveBitMap)

                    result = user32.PrintWindow(self.selected_window_handle, saveDC.GetSafeHdc(), 3)

                    if result == 1:
                        bmpstr = saveBitMap.GetBitmapBits(True)
                        frame = Image.frombuffer('RGB', (w, h), bmpstr, 'raw', 'BGRX', 0, 1)
                        self.frames.append(frame)

                    win32gui.DeleteObject(saveBitMap.GetHandle())
                    saveDC.DeleteDC()
                    mfcDC.DeleteDC()
                    win32gui.ReleaseDC(self.selected_window_handle, hwndDC)

                elif self.record_mode.get() == "area" and bbox:
                    frame = ImageGrab.grab(bbox)
                    self.frames.append(frame)

                time.sleep(interval_ms)

            except Exception as e:
                print(f"Error capturing frame: {e}")
                self.stop_recording()
                break

    def save_gif(self):
        if not self.frames:
            messagebox.showinfo("Info", "No frames recorded.")
            return

        file_path = filedialog.asksaveasfilename(defaultextension=".gif", filetypes=[("GIF files", "*.gif")])
        if file_path:
            self.frames[0].save(
                file_path,
                save_all=True,
                append_images=self.frames[1:],
                optimize=False,
                duration=int(self.interval_entry.get()),
                loop=0
            )
            messagebox.showinfo("Success", f"GIF saved to {file_path}")


class AreaSelector(tk.Toplevel):
    def __init__(self, master):
        super().__init__(master)
        self.master = master
        self.attributes("-fullscreen", True)
        self.attributes("-alpha", 0.3)
        self.bind("<Button-1>", self.on_mouse_down)
        self.bind("<B1-Motion>", self.on_mouse_move)
        self.bind("<ButtonRelease-1>", self.on_mouse_up)
        self.rect = None
        self.start_x = None
        self.start_y = None
        self.canvas = tk.Canvas(self, cursor="cross")
        self.canvas.pack(fill=tk.BOTH, expand=True)

    def on_mouse_down(self, event):
        self.start_x = self.winfo_pointerx()
        self.start_y = self.winfo_pointery()
        if not self.rect:
            self.rect = self.canvas.create_rectangle(self.start_x, self.start_y, self.start_x, self.start_y, outline='red', width=2)

    def on_mouse_move(self, event):
        cur_x = self.winfo_pointerx()
        cur_y = self.winfo_pointery()
        self.canvas.coords(self.rect, self.start_x, self.start_y, cur_x, cur_y)

    def on_mouse_up(self, event):
        x1 = min(self.start_x, self.winfo_pointerx())
        y1 = min(self.start_y, self.winfo_pointery())
        x2 = max(self.start_x, self.winfo_pointerx())
        y2 = max(self.start_y, self.winfo_pointery())
        self.destroy()
        self.master.deiconify()
        
        bbox = (x1, y1, x2, y2)
        record_thread = threading.Thread(target=self.master.capture_frames, args=(bbox,))
        record_thread.daemon = True
        record_thread.start()


class WindowSelector(tk.Toplevel):
    def __init__(self, master):
        super().__init__(master)
        self.master = master
        self.title("选择窗口")
        self.geometry("300x200")

        self.listbox = tk.Listbox(self)
        self.listbox.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        self.listbox.bind("<Double-Button-1>", self.select_window)
        self.populate_windows()

        select_button = tk.Button(self, text="Confirm", command=self.select_window)
        select_button.pack(pady=5)

    def get_windows(self):
        windows = []
        def callback(hwnd, _):
            if win32gui.IsWindowVisible(hwnd) and win32gui.GetWindowText(hwnd):
                windows.append((hwnd, win32gui.GetWindowText(hwnd)))
        win32gui.EnumWindows(callback, None)
        return windows

    def populate_windows(self):
        self.windows = self.get_windows()
        for _, title in self.windows:
            self.listbox.insert(tk.END, title)

    def select_window(self, event=None):
        selected_index = self.listbox.curselection()
        if not selected_index:
            messagebox.showwarning("Warning", "Please select a window.")
            return
        
        self.master.selected_window_handle = self.windows[selected_index[0]][0]
        self.destroy()
        self.master.deiconify()

        record_thread = threading.Thread(target=self.master.capture_frames)
        record_thread.daemon = True
        record_thread.start()


if __name__ == "__main__":
    app = GifRecorder()
    app.mainloop()
