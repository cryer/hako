import sys
import json
import numpy as np
import cv2
import mediapipe as mp
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout,
    QHBoxLayout, QPushButton, QLabel, QRadioButton, QButtonGroup
)
from PyQt5.QtCore import QTimer, Qt, QPointF, QRectF
from PyQt5.QtGui import (
    QPainter, QPen, QBrush, QColor, QPainterPath, QFont,
    QRadialGradient
)
import os
from datetime import datetime


mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
HAND_CONNECTIONS = mp_hands.HAND_CONNECTIONS


class HandCaptureWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.cap = cv2.VideoCapture(0)
        self.hands = mp_hands.Hands(
            static_image_mode=False,
            max_num_hands=1,  
            min_detection_confidence=0.7,
            min_tracking_confidence=0.7
        )
        self.recording = False
        self.recorded_data = []
        self.display_mode = "full"  

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_frame)
        self.timer.start(30)

        self.setFixedSize(640, 480)
        self.image = None
        self.current_landmarks = None

    def set_display_mode(self, mode):
        self.display_mode = mode

    def update_frame(self):
        ret, frame = self.cap.read()
        if not ret:
            return

        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = self.hands.process(rgb_frame)

        self.current_landmarks = None
        if results.multi_hand_landmarks:
            hand_landmarks = results.multi_hand_landmarks[0]
            self.current_landmarks = hand_landmarks

            if self.recording:
                landmarks_list = []
                for lm in hand_landmarks.landmark:
                    landmarks_list.append({
                        'x': lm.x,
                        'y': lm.y,
                        'z': lm.z
                    })
                self.recorded_data.append(landmarks_list)

            if self.display_mode == "full":
                mp_drawing.draw_landmarks(
                    rgb_frame,
                    hand_landmarks,
                    mp_hands.HAND_CONNECTIONS,
                    mp_drawing.DrawingSpec(color=(255, 200, 150), thickness=2, circle_radius=4), 
                    mp_drawing.DrawingSpec(color=(180, 120, 80), thickness=2) 
                )

        display_img = rgb_frame.copy()
        if self.display_mode == "hand_only":
            display_img = np.zeros_like(rgb_frame)
            if self.current_landmarks:
                self.draw_hand_on_image(display_img, self.current_landmarks)

        h, w, ch = display_img.shape
        bytes_per_line = ch * w
        self.image = self.rgb2qpixmap(display_img, w, h, bytes_per_line)
        self.update()

    def draw_hand_on_image(self, img, landmarks):
        h, w = img.shape[:2]
        for connection in HAND_CONNECTIONS:
            start_idx, end_idx = connection
            start = landmarks.landmark[start_idx]
            end = landmarks.landmark[end_idx]
            x1, y1 = int(start.x * w), int(start.y * h)
            x2, y2 = int(end.x * w), int(end.y * h)
            cv2.line(img, (x1, y1), (x2, y2), (255, 200, 150), 3)

        for lm in landmarks.landmark:
            x, y = int(lm.x * w), int(lm.y * h)
            cv2.circle(img, (x, y), 6, (255, 255, 255), -1)

    def rgb2qpixmap(self, rgb_img, width, height, bytes_per_line):
        from PyQt5.QtGui import QImage, QPixmap
        qimg = QImage(rgb_img.data, width, height, bytes_per_line, QImage.Format_RGB888)
        return QPixmap.fromImage(qimg)

    def paintEvent(self, event):
        if self.image:
            painter = QPainter(self)
            painter.drawPixmap(self.rect(), self.image)

    def start_recording(self):
        self.recording = True
        self.recorded_data = []

    def stop_recording(self):
        self.recording = False
        if not self.recorded_data:
            return None
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"hand_gesture_{timestamp}.json"
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(self.recorded_data, f, indent=2)
        print(f"手势数据已保存至: {filename}")
        return filename

    def release(self):
        self.timer.stop()
        self.cap.release()
        self.hands.close()


class HandReplayWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.setFixedSize(640, 480)
        self.landmarks_sequence = []
        self.current_frame = 0
        self.playing = False
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.next_frame)
        self.connections = HAND_CONNECTIONS

    def load_latest_gesture(self):
        json_files = [f for f in os.listdir('.') if f.startswith('hand_gesture_') and f.endswith('.json')]
        if not json_files:
            return False
        latest_file = max(json_files, key=os.path.getctime)
        with open(latest_file, 'r', encoding='utf-8') as f:
            self.landmarks_sequence = json.load(f)
        self.current_frame = 0
        return True

    def start_replay(self):
        if not self.landmarks_sequence:
            if not self.load_latest_gesture():
                return
        self.playing = True
        self.current_frame = 0
        self.timer.start(50)

    def next_frame(self):
        if self.current_frame >= len(self.landmarks_sequence):
            self.timer.stop()
            self.playing = False
            self.current_frame = 0
            self.update()
            return
        self.update()
        self.current_frame += 1

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)

        painter.fillRect(self.rect(), QColor(20, 20, 30))

        if not self.landmarks_sequence or self.current_frame >= len(self.landmarks_sequence):
            painter.setPen(QColor(200, 200, 200))
            font = QFont()
            font.setPointSize(16)
            painter.setFont(font)
            painter.drawText(self.rect(), Qt.AlignCenter, "点击“模拟”加载手势回放")
            return

        landmarks = self.landmarks_sequence[self.current_frame]
        w, h = self.width(), self.height()
        points = []
        for lm in landmarks:
            x = (1 - lm['x']) * w  
            y = lm['y'] * h
            points.append(QPointF(x, y))

        if len(points) < 21:
            return

        palm_indices = [0, 1, 2, 5, 9, 13, 17, 0]
        palm_path = QPainterPath()
        palm_path.moveTo(points[0])
        for idx in palm_indices[1:]:
            palm_path.lineTo(points[idx])
        palm_path.closeSubpath()

        center = points[0]  
        gradient = QRadialGradient(center, 200)
        gradient.setColorAt(0, QColor(230, 190, 150))
        gradient.setColorAt(1, QColor(200, 160, 120))
        painter.fillPath(palm_path, QBrush(gradient))

        shadow_offset = QPointF(3, 3)
        painter.setPen(QPen(QColor(0, 0, 0, 80), 5))
        for connection in self.connections:
            start_idx, end_idx = connection
            if start_idx < len(points) and end_idx < len(points):
                p1 = points[start_idx] + shadow_offset
                p2 = points[end_idx] + shadow_offset
                painter.drawLine(p1, p2)

        painter.setPen(QPen(QColor(180, 120, 80), 4))
        for connection in self.connections:
            start_idx, end_idx = connection
            if start_idx < len(points) and end_idx < len(points):
                painter.drawLine(points[start_idx], points[end_idx])

        painter.setPen(Qt.NoPen)
        for i, pt in enumerate(points):
            joint_color = QColor(220, 170, 130)
            if i in [4, 8, 12, 16, 20]:  
                joint_color = QColor(240, 200, 170)
            painter.setBrush(QBrush(joint_color))
            painter.drawEllipse(pt, 7, 7)

        fingertips = [4, 8, 12, 16, 20]
        nail_color = QColor(255, 250, 245)
        for idx in fingertips:
            if idx < len(points):
                tip = points[idx]
                prev_idx = idx - 1
                if prev_idx >= 0:
                    direction = tip - points[prev_idx]
                    length = (direction.x()**2 + direction.y()**2)**0.5
                    if length > 0:
                        dx = direction.x() / length
                        dy = direction.y() / length
                    else:
                        dx, dy = 0, -1
                else:
                    dx, dy = 0, -1

                painter.save()
                painter.translate(tip.x(), tip.y())
                angle = np.degrees(np.arctan2(dy, dx))
                painter.rotate(angle)

                nail_rect = QRectF(-6, -3, 12, 6)
                painter.setBrush(QBrush(nail_color))
                painter.setPen(QPen(QColor(230, 220, 210), 1))
                painter.drawEllipse(nail_rect)
                painter.restore()


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("手势捕获与回放系统")
        self.setGeometry(100, 100, 1320, 600)

        self.capture_widget = HandCaptureWidget()
        self.replay_widget = HandReplayWidget()

        self.btn_record = QPushButton("录制")
        self.btn_stop = QPushButton("停止录制")
        self.btn_replay = QPushButton("模拟")

        self.radio_full = QRadioButton("完整画面")
        self.radio_hand_only = QRadioButton("仅手部")
        self.radio_full.setChecked(True)
        self.radio_group = QButtonGroup()
        self.radio_group.addButton(self.radio_full)
        self.radio_group.addButton(self.radio_hand_only)

        self.radio_full.toggled.connect(self.on_radio_toggled)
        self.btn_record.clicked.connect(self.start_recording)
        self.btn_stop.clicked.connect(self.stop_recording)
        self.btn_replay.clicked.connect(self.replay_latest)

        for btn in [self.btn_record, self.btn_stop, self.btn_replay]:
            btn.setStyleSheet("font-size: 14px; padding: 6px;")

        top_layout = QHBoxLayout()
        top_layout.addWidget(QLabel("摄像头画面"))
        top_layout.addWidget(self.radio_full)
        top_layout.addWidget(self.radio_hand_only)

        display_layout = QHBoxLayout()
        display_layout.addWidget(self.capture_widget)
        display_layout.addWidget(self.replay_widget)

        control_layout = QHBoxLayout()
        control_layout.addWidget(self.btn_record)
        control_layout.addWidget(self.btn_stop)
        control_layout.addWidget(self.btn_replay)

        main_layout = QVBoxLayout()
        main_layout.addLayout(top_layout)
        main_layout.addLayout(display_layout)
        main_layout.addLayout(control_layout)

        container = QWidget()
        container.setLayout(main_layout)
        self.setCentralWidget(container)

    def on_radio_toggled(self):
        if self.radio_full.isChecked():
            self.capture_widget.set_display_mode("full")
        else:
            self.capture_widget.set_display_mode("hand_only")

    def start_recording(self):
        self.capture_widget.start_recording()
        print("开始录制手势...")

    def stop_recording(self):
        filename = self.capture_widget.stop_recording()
        if filename:
            print("录制已停止并保存。")

    def replay_latest(self):
        self.replay_widget.start_replay()

    def closeEvent(self, event):
        self.capture_widget.release()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
