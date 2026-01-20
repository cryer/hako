import math
from PyQt5.QtWidgets import QWidget
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QPainter, QColor, QPen, QFont, QRadialGradient


class CarGauge(QWidget):
    def __init__(self, parent=None, theme='classic', min_value=0, max_value=220, label="SPEED"):
        super().__init__(parent)
        self.setFixedSize(250, 250)
        
        self.themes = {
            'classic': {
                'background': QColor(20, 20, 30),
                'border': QColor(80, 80, 100),
                'needle': QColor(255, 50, 50),
                'scale_text': QColor(220, 220, 240),
                'scale_lines': QColor(180, 180, 200),
                'value_text': QColor(255, 200, 100),
                'label_text': QColor(180, 200, 220)
            },
            'sport': {
                'background': QColor(15, 15, 25),
                'border': QColor(100, 50, 50),
                'needle': QColor(255, 80, 80),
                'scale_text': QColor(240, 200, 200),
                'scale_lines': QColor(200, 150, 150),
                'value_text': QColor(255, 150, 150),
                'label_text': QColor(220, 180, 180)
            },
            'luxury': {
                'background': QColor(25, 20, 20),
                'border': QColor(120, 100, 80),
                'needle': QColor(255, 215, 0),
                'scale_text': QColor(240, 220, 200),
                'scale_lines': QColor(200, 180, 160),
                'value_text': QColor(255, 220, 180),
                'label_text': QColor(220, 200, 180)
            },
            'racing': {
                'background': QColor(10, 10, 15),
                'border': QColor(60, 180, 60),
                'needle': QColor(60, 220, 60),
                'scale_text': QColor(200, 255, 200),
                'scale_lines': QColor(150, 220, 150),
                'value_text': QColor(180, 255, 180),
                'label_text': QColor(200, 240, 200)
            }
        }
        
        self.current_theme = theme if theme in self.themes else 'classic'
        self.theme_colors = self.themes[self.current_theme]
        
        self.min_value = min_value
        self.max_value = max_value
        self.current_value = min_value
        self.label = label
        
        self.target_value = min_value
        self.animation_step = 0
        self.animation_timer = QTimer(self)
        self.animation_timer.timeout.connect(self.animate_needle)
        self.animation_timer.setInterval(16)  
        
    def set_value(self, value):
        clamped_value = max(self.min_value, min(self.max_value, value))
        self.target_value = clamped_value
        
        if not self.animation_timer.isActive():
            self.animation_timer.start()
    
    def animate_needle(self):
        diff = self.target_value - self.current_value
        if abs(diff) < 0.1:
            self.current_value = self.target_value
            self.animation_timer.stop()
        else:
            self.current_value += diff * 0.1  
        
        self.update()
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)
        
        rect = self.rect()
        center = rect.center()
        radius = min(rect.width(), rect.height()) // 2 - 10
        
        self.draw_gauge_background(painter, center, radius)
        
        self.draw_scale(painter, center, radius)
        
        self.draw_needle(painter, center, radius)
        
        self.draw_value_and_label(painter, center, radius)
    
    def draw_gauge_background(self, painter, center, radius):
        outer_radius = radius + 8
        border_grad = QRadialGradient(center, outer_radius * 1.2, center)
        border_grad.setColorAt(0.0, self.theme_colors['border'].lighter(150))
        border_grad.setColorAt(0.7, self.theme_colors['border'])
        border_grad.setColorAt(1.0, self.theme_colors['border'].darker(150))
        
        painter.setBrush(border_grad)
        painter.setPen(Qt.NoPen)
        painter.drawEllipse(center, outer_radius, outer_radius)
        
        background_grad = QRadialGradient(center, radius * 1.2, center)
        background_grad.setColorAt(0.0, self.theme_colors['background'].lighter(120))
        background_grad.setColorAt(1.0, self.theme_colors['background'])
        
        painter.setBrush(background_grad)
        painter.drawEllipse(center, radius, radius)
    
    def draw_scale(self, painter, center, radius):
        start_angle = -150
        end_angle = 150
        total_angle = end_angle - start_angle
        
        main_ticks = 10
        for i in range(main_ticks + 1):
            angle = start_angle + (i * total_angle / main_ticks)
            value = self.min_value + (i * (self.max_value - self.min_value) / main_ticks)
            
            rad = math.radians(angle - 90)
            outer_x = center.x() + (radius - 10) * math.cos(rad)
            outer_y = center.y() + (radius - 10) * math.sin(rad)
            
            inner_x = center.x() + (radius - 25) * math.cos(rad)
            inner_y = center.y() + (radius - 25) * math.sin(rad)
            
            pen = QPen(self.theme_colors['scale_lines'])
            pen.setWidth(3)
            painter.setPen(pen)
            painter.drawLine(int(outer_x), int(outer_y), int(inner_x), int(inner_y))
            
            text_x = center.x() + (radius - 40) * math.cos(rad)
            text_y = center.y() + (radius - 40) * math.sin(rad)
            
            painter.setPen(self.theme_colors['scale_text'])
            painter.setFont(QFont("Arial", 10, QFont.Bold))
            painter.drawText(
                int(text_x - 15), int(text_y - 8), 30, 16,
                Qt.AlignCenter, str(int(value))
            )
        
        sub_ticks = 50
        for i in range(sub_ticks + 1):
            angle = start_angle + (i * total_angle / sub_ticks)
            if i % 5 != 0:  
                rad = math.radians(angle - 90)
                outer_x = center.x() + (radius - 15) * math.cos(rad)
                outer_y = center.y() + (radius - 15) * math.sin(rad)
                inner_x = center.x() + (radius - 22) * math.cos(rad)
                inner_y = center.y() + (radius - 22) * math.sin(rad)
                
                pen = QPen(self.theme_colors['scale_lines'])
                pen.setWidth(1)
                painter.setPen(pen)
                painter.drawLine(int(outer_x), int(outer_y), int(inner_x), int(inner_y))
    
    def draw_needle(self, painter, center, radius):
        if self.max_value == self.min_value:
            angle = -150 
        else:
            ratio = (self.current_value - self.min_value) / (self.max_value - self.min_value)
            angle = -150 + ratio * 300 
        
        rad = math.radians(angle - 90)
        
        needle_length = radius - 30
        needle_x = center.x() + needle_length * math.cos(rad)
        needle_y = center.y() + needle_length * math.sin(rad)
        
        pen = QPen(self.theme_colors['needle'])
        pen.setWidth(4)
        painter.setPen(pen)
        painter.drawLine(int(center.x()), int(center.y()), int(needle_x), int(needle_y))
        
        painter.setBrush(self.theme_colors['needle'])
        painter.setPen(Qt.NoPen)
        painter.drawEllipse(
            int(needle_x - 6), int(needle_y - 6), 12, 12
        )
        
        painter.setBrush(QColor(50, 50, 60))
        painter.drawEllipse(
            int(center.x() - 8), int(center.y() - 8), 16, 16
        )
    
    def draw_value_and_label(self, painter, center, radius):
        painter.setPen(self.theme_colors['value_text'])
        painter.setFont(QFont("Arial", 18, QFont.Bold))
        value_text = str(int(self.current_value))
        painter.drawText(
            int(center.x() - 40), int(center.y() + 10), 80, 30,
            Qt.AlignCenter, value_text
        )
        
        painter.setPen(self.theme_colors['label_text'])
        painter.setFont(QFont("Arial", 10))
        painter.drawText(
            int(center.x() - 30), int(center.y() + 40), 60, 20,
            Qt.AlignCenter, self.label
        )


def create_car_gauge(parent_window, theme='classic', min_value=0, max_value=220, label="SPEED"):
    gauge = CarGauge(parent_window, theme=theme, min_value=min_value, max_value=max_value, label=label)
    return gauge
