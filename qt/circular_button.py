import math
from PyQt5.QtWidgets import QWidget
from PyQt5.QtCore import Qt, QTimer, QRectF
from PyQt5.QtGui import QPainter, QColor, QRadialGradient, QFont, QPainterPath
from PyQt5.QtCore import pyqtSignal

COLOR_THEMES = {
    'blue': {
        'sphere_center': QColor(80, 120, 180, 255),
        'sphere_mid': QColor(40, 80, 140, 255),
        'sphere_edge': QColor(20, 50, 90, 255),
        'wave_normal': QColor(30, 100, 180, 180),
        'wave_hover': QColor(60, 140, 220, 200),
        'wave_progress': QColor(0, 160, 220, 220)
    },
    'green': {
        'sphere_center': QColor(80, 180, 120, 255),
        'sphere_mid': QColor(40, 140, 80, 255),
        'sphere_edge': QColor(20, 90, 50, 255),
        'wave_normal': QColor(30, 180, 100, 180),
        'wave_hover': QColor(60, 220, 140, 200),
        'wave_progress': QColor(0, 220, 160, 220)
    },
    'pink': {
        'sphere_center': QColor(180, 80, 120, 255),
        'sphere_mid': QColor(140, 40, 80, 255),
        'sphere_edge': QColor(90, 20, 50, 255),
        'wave_normal': QColor(180, 30, 100, 180),
        'wave_hover': QColor(220, 60, 140, 200),
        'wave_progress': QColor(220, 0, 160, 220)
    },
    'purple': {
        'sphere_center': QColor(120, 80, 180, 255),
        'sphere_mid': QColor(80, 40, 140, 255),
        'sphere_edge': QColor(50, 20, 90, 255),
        'wave_normal': QColor(100, 30, 180, 180),
        'wave_hover': QColor(140, 60, 220, 200),
        'wave_progress': QColor(160, 0, 220, 220)
    }
}


class WaveEffect:
    def __init__(self):
        self.offset = 0                   
        self.amplitude = 4               
        self.wavelength = 35              
        self.speed = 1                
        
    def update(self):
        self.offset += self.speed
        if self.offset > self.wavelength:
            self.offset = 0
            
    def get_wave_points(self, rect, water_level):
        points = []
        width = rect.width()
        height = rect.height()
        center_x = rect.center().x()
        center_y = rect.center().y()
        
        water_y = center_y + (height / 2) - (height * water_level)
        
        start_x = int(center_x - width/2)
        end_x = int(center_x + width/2 + 1)
        
        for x in range(start_x, end_x, 2):
            wave_height = self.amplitude * math.sin(2 * math.pi * (x - self.offset) / self.wavelength)
            y = water_y + wave_height
            points.append((x, y))
            
        return points


class CircularButton(QWidget):
    clicked = pyqtSignal()
    
    def __init__(self, parent=None, theme='blue'):
        super().__init__(parent)
        self.setFixedSize(200, 200)     
        self.setMouseTracking(True)     
        
        self._hovered = False            
        self._pressed = False             
        self._progress = 0                
        self._operation_running = False    
        
        self.wave_effect = WaveEffect()
        
        self.wave_timer = QTimer(self)
        self.wave_timer.timeout.connect(self.update_wave)
        self.wave_timer.start(50)
      
        self._current_theme = theme
        self._apply_theme(theme)

        self._initial_text = "开始"
        self._auto_progress_timer = None

    def _apply_theme(self, theme_name):
        theme = COLOR_THEMES.get(theme_name, COLOR_THEMES['blue'])
        self.normal_wave_color = theme['wave_normal']
        self.hover_wave_color = theme['wave_hover']
        self.progress_wave_color = theme['wave_progress']
        self.sphere_colors = {
            'center': theme['sphere_center'],
            'mid': theme['sphere_mid'],
            'edge': theme['sphere_edge']
        }

    def set_initial_text(self, text):
        self._initial_text = text
        self.update()
    
    def update_wave(self):
        self.wave_effect.update()
        self.update()

    def start_progress(self):
        self._operation_running = True
        self._progress = 0
        self.update()

    def start_auto_progress(self, duration_ms=3000):
        self._operation_running = True
        self._progress = 0
        self.update()
        
        total_steps = 100
        interval = duration_ms // total_steps
        
        self._auto_progress_timer = QTimer(self)
        self._auto_progress_timer.timeout.connect(self._auto_progress_step)
        self._auto_progress_timer.start(interval)

    def set_progress_value(self, value):
        if self._operation_running:
            self._progress = max(0, min(100, value))
            self.update()
            if self._progress >= 100:
                self._operation_running = False
                self._progress = 0

    def _auto_progress_step(self):
        self._progress += 1
        if self._progress >= 100:
            self._progress = 100
            self._auto_progress_timer.stop()
            self._auto_progress_timer = None
            self._operation_running = False
        self.update()

    def finish_progress(self):
        if self._auto_progress_timer:
            self._auto_progress_timer.stop()
            self._auto_progress_timer = None
        self.set_progress_value(100)
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)      
        painter.setRenderHint(QPainter.SmoothPixmapTransform)  
        rect = self.rect()
        center = rect.center()
        radius = min(rect.width(), rect.height()) // 2 - 2 
        
        self.draw_sphere_effect(painter, center, radius)
        
        if self._operation_running:
            self.draw_progress_water(painter, center, radius)
        else:
            self.draw_wave_effect(painter, center, radius)
    
    def draw_sphere_effect(self, painter, center, radius):
        radial_grad = QRadialGradient(center, radius * 1.2, center)
        radial_grad.setColorAt(0.0, self.sphere_colors['center'])
        radial_grad.setColorAt(0.6, self.sphere_colors['mid'])
        radial_grad.setColorAt(1.0, self.sphere_colors['edge'])
        
        painter.setBrush(radial_grad)
        painter.setPen(Qt.NoPen)
        painter.drawEllipse(int(center.x() - radius), int(center.y() - radius), 
                           int(radius * 2), int(radius * 2))
        
        highlight_x = int(center.x() - radius * 0.3)
        highlight_y = int(center.y() - radius * 0.3)
        highlight_radius = int(radius * 0.2)
        
        highlight_grad = QRadialGradient(highlight_x, highlight_y, highlight_radius, highlight_x, highlight_y)
        highlight_grad.setColorAt(0.0, QColor(255, 255, 255, 120))
        highlight_grad.setColorAt(1.0, QColor(255, 255, 255, 0))
        
        painter.setBrush(highlight_grad)
        painter.drawEllipse(highlight_x - highlight_radius, highlight_y - highlight_radius,
                           highlight_radius * 2, highlight_radius * 2)
    
    def draw_wave_effect(self, painter, center, radius):
        wave_color = self.hover_wave_color if self._hovered else self.normal_wave_color
        
        inner_radius = radius - 8
        wave_rect = QRectF(
            center.x() - inner_radius,
            center.y() - inner_radius,
            inner_radius * 2,
            inner_radius * 2
        )
        
        clip_path = QPainterPath()
        clip_path.addEllipse(center, inner_radius, inner_radius)
        painter.setClipPath(clip_path)
        
        points = self.wave_effect.get_wave_points(wave_rect, 0.3)
        if points:
            from PyQt5.QtGui import QPolygonF
            from PyQt5.QtCore import QPointF
            
            polygon_points = []
            for x, y in points:
                polygon_points.append(QPointF(x, y))
            
            right_x = points[-1][0] if points else wave_rect.right()
            left_x = points[0][0] if points else wave_rect.left()
            bottom_y = wave_rect.bottom()
            
            polygon_points.append(QPointF(right_x, bottom_y))
            polygon_points.append(QPointF(left_x, bottom_y))
            
            wave_polygon = QPolygonF(polygon_points)
            
            painter.setBrush(wave_color)
            painter.setPen(Qt.NoPen)
            painter.drawPolygon(wave_polygon)
        
        if not self._operation_running:
            painter.setClipping(False) 
            painter.setPen(QColor(255, 255, 255))
            painter.setFont(QFont("Arial", 14, QFont.Bold))
            text_rect = painter.boundingRect(self.rect(), Qt.AlignCenter, self._initial_text)
            painter.drawText(text_rect, Qt.AlignCenter, self._initial_text)
        painter.setClipping(False)
    
    def draw_progress_water(self, painter, center, radius):
        water_level = self._progress / 100.0
        
        inner_radius = radius - 8
        water_rect = QRectF(
            center.x() - inner_radius,
            center.y() - inner_radius,
            inner_radius * 2,
            inner_radius * 2
        )
        
        clip_path = QPainterPath()
        clip_path.addEllipse(center, inner_radius, inner_radius)
        painter.setClipPath(clip_path)
        
        if water_level > 0:
            points = self.wave_effect.get_wave_points(water_rect, water_level)
            if points:
                from PyQt5.QtGui import QPolygonF
                from PyQt5.QtCore import QPointF
                
                polygon_points = []
                for x, y in points:
                    polygon_points.append(QPointF(x, y))
                
                right_x = points[-1][0] if points else water_rect.right()
                left_x = points[0][0] if points else water_rect.left()
                bottom_y = center.y() + inner_radius
                
                polygon_points.append(QPointF(right_x, bottom_y))
                polygon_points.append(QPointF(left_x, bottom_y))
                
                water_polygon = QPolygonF(polygon_points)
                
                painter.setBrush(self.progress_wave_color)
                painter.setPen(Qt.NoPen)
                painter.drawPolygon(water_polygon)
        
        if self._progress > 0:
            painter.setPen(QColor(255, 255, 255))
            painter.setFont(QFont("Arial", 16, QFont.Bold))
            progress_text = f"{self._progress}%"
            text_rect = painter.boundingRect(self.rect(), Qt.AlignCenter, progress_text)
            painter.drawText(text_rect, Qt.AlignCenter, progress_text)
        
        painter.setClipping(False)
    
    def enterEvent(self, event):
        self._hovered = True
        self.update()
        super().enterEvent(event)
    
    def leaveEvent(self, event):
        self._hovered = False
        self.update()
        super().leaveEvent(event)
    
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self._pressed = True
            self.update()
        super().mousePressEvent(event)
    
    def mouseReleaseEvent(self, event):
        if event.button() == Qt.LeftButton and self._pressed:
            self._pressed = False
            self.clicked.emit()
        super().mouseReleaseEvent(event)
       

def _wrapped_callback(callback_func, button):
    callback_func()


def create_circular_button(parent_window, callback_func, initial_text="开始", theme='blue'):
    button = CircularButton(parent_window, theme=theme)
    button.set_initial_text(initial_text)
    button.clicked.connect(lambda: _wrapped_callback(callback_func, button))
    return button
