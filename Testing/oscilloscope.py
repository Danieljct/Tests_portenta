#!/usr/bin/env python3
"""
Osciloscopio PyQt5 para datos del sensor SM4291
Visualiza datos en tiempo real desde COM9
"""

import sys
import serial
import numpy as np
from collections import deque
from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, 
                            QWidget, QPushButton, QComboBox, QLabel, QSpinBox,
                            QCheckBox, QGroupBox, QGridLayout, QSlider)
from PyQt5.QtCore import QTimer, QThread, pyqtSignal, Qt
from PyQt5.QtGui import QFont
import pyqtgraph as pg
import time

class SerialOscilloscope(QThread):
    """Thread para leer datos del puerto serie"""
    new_data_point = pyqtSignal(float, float)  # timestamp, value
    status_update = pyqtSignal(str)
    
    def __init__(self):
        super().__init__()
        self.serial_port = None
        self.running = False
        self.port_name = "COM9"
        self.baud_rate = 2000000
        self.start_time = time.time()
        
    def set_port(self, port_name, baud_rate):
        """Configurar puerto serie"""
        self.port_name = port_name
        self.baud_rate = baud_rate
        
    def connect_serial(self):
        """Conectar al puerto serie"""
        try:
            self.serial_port = serial.Serial(self.port_name, self.baud_rate, timeout=0.1)
            self.start_time = time.time()
            self.status_update.emit(f"Conectado a {self.port_name}")
            return True
        except Exception as e:
            self.status_update.emit(f"Error: {str(e)}")
            return False
    
    def disconnect_serial(self):
        """Desconectar puerto serie"""
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.status_update.emit("Desconectado")
    
    def start_reading(self):
        """Iniciar lectura"""
        if self.connect_serial():
            self.running = True
            self.start()
    
    def stop_reading(self):
        """Detener lectura"""
        self.running = False
        self.disconnect_serial()
        
    def run(self):
        """Loop principal del thread"""
        while self.running and self.serial_port and self.serial_port.is_open:
            try:
                line = self.serial_port.readline().decode('utf-8').strip()
                
                if line and not line.startswith("=") and not line.startswith("Timer"):
                    try:
                        # Intentar convertir a float (datos del sensor)
                        value = float(line)
                        timestamp = time.time() - self.start_time
                        self.new_data_point.emit(timestamp, value)
                    except ValueError:
                        # Ignorar líneas que no son números
                        if "ERROR" not in line:
                            pass
                        
            except Exception as e:
                self.status_update.emit(f"Error leyendo: {str(e)}")
                break
                
        self.disconnect_serial()

class OscilloscopeWindow(QMainWindow):
    """Ventana principal del osciloscopio"""
    
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Osciloscopio PyQt5 - Sensor SM4291")
        self.setGeometry(100, 100, 1200, 800)
        
        # Variables para datos
        self.max_points = 10000  # Máximo 10000 puntos en pantalla
        self.time_data = deque(maxlen=self.max_points)
        self.value_data = deque(maxlen=self.max_points)
        
        # Variables de control
        self.is_running = False
        self.auto_scale = True
        self.trigger_enabled = False
        self.trigger_level = 1013.0
        
        # Configurar interfaz
        self.setup_ui()
        
        # Configurar serial reader
        self.serial_reader = SerialOscilloscope()
        self.serial_reader.new_data_point.connect(self.add_data_point)
        self.serial_reader.status_update.connect(self.update_status)
        
        # Timer para actualizar gráficos
        self.plot_timer = QTimer()
        self.plot_timer.timeout.connect(self.update_plot)
        self.plot_timer.start(50)  # Actualizar cada 50ms (20 FPS)
        
    def setup_ui(self):
        """Configurar interfaz de usuario"""
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Layout principal
        main_layout = QVBoxLayout(central_widget)
        
        # Panel de control superior
        control_panel = self.create_control_panel()
        main_layout.addWidget(control_panel)
        
        # Gráfico principal (osciloscopio)
        self.create_oscilloscope_plot()
        main_layout.addWidget(self.plot_widget)
        
        # Panel de controles del osciloscopio
        scope_controls = self.create_scope_controls()
        main_layout.addWidget(scope_controls)
        
    def create_control_panel(self):
        """Crear panel de control de conexión"""
        group = QGroupBox("Control de Conexión")
        layout = QHBoxLayout(group)
        
        # Puerto serie
        layout.addWidget(QLabel("Puerto:"))
        self.port_combo = QComboBox()
        self.port_combo.setEditable(True)
        self.port_combo.addItems(["COM9", "COM1", "COM3", "COM4", "COM5"])
        layout.addWidget(self.port_combo)
        
        # Baud rate
        layout.addWidget(QLabel("Baud Rate:"))
        self.baud_spin = QSpinBox()
        self.baud_spin.setRange(9600, 2000000)
        self.baud_spin.setValue(2000000)
        layout.addWidget(self.baud_spin)
        
        # Botones
        self.connect_btn = QPushButton("Conectar")
        self.connect_btn.clicked.connect(self.toggle_connection)
        layout.addWidget(self.connect_btn)
        
        self.clear_btn = QPushButton("Limpiar")
        self.clear_btn.clicked.connect(self.clear_data)
        layout.addWidget(self.clear_btn)
        
        # Status
        self.status_label = QLabel("Desconectado")
        self.status_label.setStyleSheet("color: red; font-weight: bold;")
        layout.addWidget(self.status_label)
        
        layout.addStretch()
        
        return group
    
    def create_oscilloscope_plot(self):
        """Crear el gráfico del osciloscopio"""
        # Configurar PyQtGraph
        pg.setConfigOptions(antialias=True)
        
        self.plot_widget = pg.PlotWidget(title="Osciloscopio - Sensor SM4291")
        self.plot_widget.setLabel('left', 'Presión (mbar)')
        self.plot_widget.setLabel('bottom', 'Tiempo (s)')
        self.plot_widget.showGrid(x=True, y=True)
        
        # Línea principal de datos
        self.data_curve = self.plot_widget.plot(pen=pg.mkPen('cyan', width=2))
        
        # Línea de trigger
        self.trigger_line = pg.InfiniteLine(angle=0, movable=True, pen=pg.mkPen('red', width=2, style=Qt.PenStyle.DashLine))
        self.trigger_line.sigPositionChanged.connect(self.trigger_moved)
        
    def create_scope_controls(self):
        """Crear controles del osciloscopio"""
        group = QGroupBox("Controles del Osciloscopio")
        layout = QGridLayout(group)
        
        # Controles de tiempo
        layout.addWidget(QLabel("Ventana de Tiempo (s):"), 0, 0)
        self.time_window_spin = QSpinBox()
        self.time_window_spin.setRange(1, 100)
        self.time_window_spin.setValue(10)
        self.time_window_spin.valueChanged.connect(self.update_time_window)
        layout.addWidget(self.time_window_spin, 0, 1)
        
        # Auto escala
        self.auto_scale_check = QCheckBox("Auto Escala Y")
        self.auto_scale_check.setChecked(True)
        self.auto_scale_check.toggled.connect(self.toggle_auto_scale)
        layout.addWidget(self.auto_scale_check, 0, 2)
        
        # Trigger
        self.trigger_check = QCheckBox("Trigger")
        self.trigger_check.toggled.connect(self.toggle_trigger)
        layout.addWidget(self.trigger_check, 1, 0)
        
        layout.addWidget(QLabel("Nivel Trigger:"), 1, 1)
        self.trigger_spin = QSpinBox()
        self.trigger_spin.setRange(-500, 63)
        self.trigger_spin.setValue(-60)
        self.trigger_spin.valueChanged.connect(self.update_trigger_level)
        layout.addWidget(self.trigger_spin, 1, 2)
        
        # Información en tiempo real
        layout.addWidget(QLabel("Valor Actual:"), 2, 0)
        self.current_value_label = QLabel("---")
        self.current_value_label.setStyleSheet("font-weight: bold; color: green;")
        layout.addWidget(self.current_value_label, 2, 1)
        
        layout.addWidget(QLabel("Frecuencia:"), 2, 2)
        self.frequency_label = QLabel("---")
        self.frequency_label.setStyleSheet("font-weight: bold; color: blue;")
        layout.addWidget(self.frequency_label, 2, 3)
        
        return group
    
    def toggle_connection(self):
        """Alternar conexión serie"""
        if not self.serial_reader.running:
            port = self.port_combo.currentText()
            baud = self.baud_spin.value()
            self.serial_reader.set_port(port, baud)
            self.serial_reader.start_reading()
            self.connect_btn.setText("Desconectar")
            self.is_running = True
        else:
            self.serial_reader.stop_reading()
            self.connect_btn.setText("Conectar")
            self.is_running = False
    
    def clear_data(self):
        """Limpiar todos los datos"""
        self.time_data.clear()
        self.value_data.clear()
        self.data_curve.setData([], [])
    
    def update_status(self, message):
        """Actualizar status"""
        self.status_label.setText(message)
        if "Conectado" in message:
            self.status_label.setStyleSheet("color: green; font-weight: bold;")
        elif "Error" in message:
            self.status_label.setStyleSheet("color: red; font-weight: bold;")
        else:
            self.status_label.setStyleSheet("color: blue; font-weight: bold;")
    
    def add_data_point(self, timestamp, value):
        """Agregar nuevo punto de datos"""
        self.time_data.append(timestamp)
        self.value_data.append(value)
        
        # Actualizar valor actual
        self.current_value_label.setText(f"{value:.3f}")
        
        # Calcular frecuencia aproximada
        if len(self.time_data) > 100:
            time_diff = self.time_data[-1] - self.time_data[-100]
            if time_diff > 0:
                freq = 99 / time_diff  # 99 muestras en time_diff segundos
                self.frequency_label.setText(f"{freq:.1f} Hz")
    
    def update_plot(self):
        """Actualizar el gráfico"""
        if len(self.time_data) == 0:
            return
        
        # Convertir a arrays numpy para mejor rendimiento
        times = np.array(self.time_data)
        values = np.array(self.value_data)
        
        # Aplicar ventana de tiempo
        time_window = self.time_window_spin.value()
        if len(times) > 0:
            current_time = times[-1]
            mask = times >= (current_time - time_window)
            times_windowed = times[mask]
            values_windowed = values[mask]
        else:
            times_windowed = times
            values_windowed = values
        
        # Actualizar datos del gráfico
        self.data_curve.setData(times_windowed, values_windowed)
        
        # Auto escala
        if self.auto_scale and len(values_windowed) > 0:
            self.plot_widget.setYRange(np.min(values_windowed), np.max(values_windowed), padding=0.1)
        
        # Actualizar rango X para seguir los datos
        if len(times_windowed) > 0:
            self.plot_widget.setXRange(times_windowed[0], times_windowed[-1], padding=0.02)
    
    def update_time_window(self):
        """Actualizar ventana de tiempo"""
        pass  # Se maneja en update_plot()
    
    def toggle_auto_scale(self, enabled):
        """Alternar auto escala"""
        self.auto_scale = enabled
    
    def toggle_trigger(self, enabled):
        """Alternar trigger"""
        self.trigger_enabled = enabled
        if enabled:
            self.plot_widget.addItem(self.trigger_line)
            self.trigger_line.setPos(self.trigger_level)
        else:
            self.plot_widget.removeItem(self.trigger_line)
    
    def trigger_moved(self):
        """Trigger movido"""
        self.trigger_level = self.trigger_line.pos()[1]
        self.trigger_spin.setValue(int(self.trigger_level))
    
    def update_trigger_level(self, level):
        """Actualizar nivel de trigger"""
        self.trigger_level = level
        if self.trigger_enabled:
            self.trigger_line.setPos(level)

def main():
    """Función principal"""
    app = QApplication(sys.argv)
    
    # Configurar estilo
    app.setStyle('Fusion')
    
    # Crear y mostrar ventana
    window = OscilloscopeWindow()
    window.show()
    
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()