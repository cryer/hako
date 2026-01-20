from PyQt5.QtWidgets import QWidget
from PyQt5.QtCore import Qt, QTimer, QRectF
from PyQt5.QtGui import QPainter, QColor, QPen, QFont, QPainterPath


class DynamicLineChart(QWidget):
    def __init__(self, parent=None, theme='blue', max_points=100, label='CPU'):
        super().__init__(parent)
        self.setFixedSize(300, 150)
        
        self.themes = {
            'blue': {
                'background': QColor(25, 35, 50),
                'grid': QColor(60, 80, 120, 120),
                'line': QColor(64, 158, 255),
                'fill': QColor(64, 158, 255, 80),
                'text': QColor(200, 220, 255)
            },
            'green': {
                'background': QColor(25, 40, 30),
                'grid': QColor(60, 120, 80, 120),
                'line': QColor(76, 175, 80),
                'fill': QColor(76, 175, 80, 80),
                'text': QColor(200, 255, 220)
            },
            'red': {
                'background': QColor(40, 25, 25),
                'grid': QColor(120, 60, 60, 120),
                'line': QColor(244, 67, 54),
                'fill': QColor(244, 67, 54, 80),
                'text': QColor(255, 220, 220)
            },
            'purple': {
                'background': QColor(35, 25, 50),
                'grid': QColor(100, 60, 120, 120),
                'line': QColor(156, 39, 176),
                'fill': QColor(156, 39, 176, 80),
                'text': QColor(240, 220, 255)
            }
        }
        
        self.current_theme = theme if theme in self.themes else 'blue'
        self.theme_colors = self.themes[self.current_theme]
        
        self.max_points = max_points
        self.data_points = []
        self.current_value = 0
        self.label = label
        
        self.animation_timer = QTimer(self)
        self.animation_timer.timeout.connect(self.update_animation)
        self.animation_timer.start(50)  # 20 FPS
        
    def update_animation(self):
        self.update()
    
    def add_data_point(self, value):
        clamped_value = max(0, min(100, value))
        self.data_points.append(clamped_value)
        
        if len(self.data_points) > self.max_points:
            self.data_points.pop(0)
        
        self.current_value = clamped_value
        self.update()
    
    def clear_data(self):
        self.data_points = []
        self.current_value = 0
        self.update()
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)
        
        painter.fillRect(self.rect(), self.theme_colors['background'])
        
        rect = self.rect()
        margin = 15
        chart_rect = QRectF(
            margin, margin,
            rect.width() - 2 * margin,
            rect.height() - 2 * margin
        )
        
        self.draw_grid(painter, chart_rect)
        
        if self.data_points:
            self.draw_line_chart(painter, chart_rect)
        
        self.draw_current_value(painter, rect)
    
    def draw_grid(self, painter, chart_rect):
        pen = QPen(self.theme_colors['grid'])
        pen.setWidth(1)
        painter.setPen(pen)
        
        for i in range(5):
            y = chart_rect.bottom() - (i * chart_rect.height() / 4)
            painter.drawLine(
                int(chart_rect.left()), int(y),
                int(chart_rect.right()), int(y)
            )
        
        if len(self.data_points) > 10:
            step = max(10, len(self.data_points) // 5)
            for i in range(0, len(self.data_points), step):
                x = chart_rect.right() - (len(self.data_points) - 1 - i) * (chart_rect.width() / max(1, len(self.data_points) - 1))
                painter.drawLine(
                    int(x), int(chart_rect.top()),
                    int(x), int(chart_rect.bottom())
                )
    
    def draw_line_chart(self, painter, chart_rect):
        if not self.data_points:
            return
        
        path = QPainterPath()
        fill_path = QPainterPath()
        
        point_count = len(self.data_points)
        if point_count == 1:
            x = chart_rect.right()
            y = chart_rect.bottom() - (self.data_points[0] / 100.0) * chart_rect.height()
            path.moveTo(x, y)
            fill_path.moveTo(x, y)
            fill_path.lineTo(x, chart_rect.bottom())
        else:
            for i, value in enumerate(self.data_points):
                x = chart_rect.right() - (point_count - 1 - i) * (chart_rect.width() / (point_count - 1))
                y = chart_rect.bottom() - (value / 100.0) * chart_rect.height()
                
                if i == 0:
                    path.moveTo(x, y)
                    fill_path.moveTo(x, y)
                else:
                    path.lineTo(x, y)
                    fill_path.lineTo(x, y)
            
            fill_path.lineTo(chart_rect.right(), chart_rect.bottom())
            fill_path.lineTo(chart_rect.left(), chart_rect.bottom())
            fill_path.lineTo(chart_rect.left(), chart_rect.bottom() - (self.data_points[0] / 100.0) * chart_rect.height())
        
        painter.setBrush(self.theme_colors['fill'])
        painter.setPen(Qt.NoPen)
        painter.drawPath(fill_path)
        
        pen = QPen(self.theme_colors['line'])
        pen.setWidth(2)
        painter.setPen(pen)
        painter.setBrush(Qt.NoBrush)
        painter.drawPath(path)
    
    def draw_current_value(self, painter, rect):
        painter.setPen(self.theme_colors['text'])
        painter.setFont(QFont("Arial", 12, QFont.Bold))
        
        value_text = f"{int(self.current_value)}%"
        value_rect = QRectF(10, 5, 80, 30)
        painter.drawText(value_rect, Qt.AlignLeft | Qt.AlignVCenter, value_text)
        
        label_text = self.label
        label_rect = QRectF(rect.width() - 60, 5, 60, 30)
        painter.drawText(label_rect, Qt.AlignRight | Qt.AlignVCenter, label_text)


def create_dynamic_line_chart(parent_window, theme='blue', max_points=100, label='CPU'):
    chart = DynamicLineChart(parent_window, theme=theme, max_points=max_points, label=label)
    return chart
