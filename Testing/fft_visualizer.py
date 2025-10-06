#!/usr/bin/env python3
"""
FFT Visualizer para datos de sensores Portenta H7
Visualiza la FFT en tiempo real desde el puerto serie
"""

import sys
import serial
import numpy as np
from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, 
                            QWidget, QPushButton, QComboBox, QLabel, QSpinBox,
                            QCheckBox, QGroupBox, QGridLayout, QTextEdit, QSplitter)
from PyQt5.QtCore import QTimer, QThread, pyqtSignal, Qt
from PyQt5.QtGui import QFont
import pyqtgraph as pg
import time

class SerialReader(QThread):
    """Thread para leer datos del puerto serie"""
    new_data = pyqtSignal(str, list)  # sensor_name, fft_data
    status_update = pyqtSignal(str)
    
    def __init__(self):
        super().__init__()
        self.serial_port = None
        self.running = False
        self.port_name = "COM9"
        self.baud_rate = 2000000
        
    def set_port(self, port_name, baud_rate):
        """Configurar puerto serie"""
        self.port_name = port_name
        self.baud_rate = baud_rate
        
    def connect_serial(self):
        """Conectar al puerto serie"""
        try:
            self.serial_port = serial.Serial(self.port_name, self.baud_rate, timeout=1)
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
        current_sensor = "ABPLLN"  # Empezar con ABPLLN por defecto
        fft_data = []
        
        while self.running and self.serial_port and self.serial_port.is_open:
            try:
                line = self.serial_port.readline().decode('utf-8').strip()
                
                if line.startswith("SENSOR:"):
                    # Nueva captura de sensor
                    if fft_data:  # Si tenemos datos previos, enviarlos
                        print(f"DEBUG: Enviando {len(fft_data)} datos de {current_sensor}")
                        self.new_data.emit(current_sensor, fft_data.copy())
                        fft_data = []  # Limpiar después de enviar
                    
                    current_sensor = line.split(":")[1]
                    print(f"DEBUG: Cambiando a sensor {current_sensor}")
                    self.status_update.emit(f"Leyendo sensor: {current_sensor}")
                    
                elif line and not line.startswith("=") and line:
                    # Datos de FFT (números)
                    try:
                        value = float(line)
                        fft_data.append(value)
                        if len(fft_data) % 500 == 0:  # Debug cada 500 muestras
                            print(f"DEBUG: {len(fft_data)} muestras de {current_sensor}")
                        
                        # Enviar cuando tengamos 2048 muestras (FFT completa)
                        if len(fft_data) >= 2048:
                            print(f"DEBUG: FFT completa! Enviando {len(fft_data)} datos de {current_sensor}")
                            self.new_data.emit(current_sensor, fft_data.copy())
                            fft_data = []  # Limpiar para siguiente FFT
                            
                    except ValueError:
                        if line:  # Solo mostrar si no está vacío
                            print(f"DEBUG: Línea ignorada: '{line}'")
                        
            except Exception as e:
                self.status_update.emit(f"Error leyendo: {str(e)}")
                print(f"DEBUG: Error: {e}")
                break
                
        self.disconnect_serial()

class FFTVisualizer(QMainWindow):
    """Ventana principal de la aplicación"""
    
    def __init__(self):
        super().__init__()
        self.setWindowTitle("FFT Visualizer - Portenta H7 Sensors")
        self.setGeometry(100, 100, 1400, 800)
        
        # Variables
        self.fft_size = 4096
        self.sample_rate = 2000
        self.current_sensor = "ABPLLN"
        self.sensor_data = {
            "ABPLLN": {"fft": [], "color": "red"},
            "ELV": {"fft": [], "color": "blue"}, 
            "SM4291": {"fft": [], "color": "green"},
            "CCDANN600": {"fft": [], "color": "orange"}
        }
        
        # Configurar interfaz
        self.setup_ui()
        
        # Configurar serial reader
        self.serial_reader = SerialReader()
        self.serial_reader.new_data.connect(self.update_fft_data)
        self.serial_reader.status_update.connect(self.update_status)
        
        # Timer para actualizar gráficos
        self.plot_timer = QTimer()
        self.plot_timer.timeout.connect(self.update_plots)
        self.plot_timer.start(100)  # Actualizar cada 100ms
        
    def setup_ui(self):
        """Configurar interfaz de usuario"""
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Layout principal
        main_layout = QVBoxLayout(central_widget)
        
        # Panel de control
        control_panel = self.create_control_panel()
        main_layout.addWidget(control_panel)
        
        # Splitter para gráficos y log
        splitter = QSplitter(Qt.Orientation.Horizontal)
        
        # Panel de gráficos
        plot_widget = self.create_plot_panel()
        splitter.addWidget(plot_widget)
        
        # Panel de log
        log_widget = self.create_log_panel()
        splitter.addWidget(log_widget)
        
        # Configurar tamaños del splitter
        splitter.setSizes([1000, 400])
        main_layout.addWidget(splitter)
        
    def create_control_panel(self):
        """Crear panel de control"""
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
        self.baud_spin.setRange(9600, 921600)
        self.baud_spin.setValue(115200)
        layout.addWidget(self.baud_spin)
        
        # Botones
        self.connect_btn = QPushButton("Conectar")
        self.connect_btn.clicked.connect(self.toggle_connection)
        layout.addWidget(self.connect_btn)
        
        self.clear_btn = QPushButton("Limpiar Datos")
        self.clear_btn.clicked.connect(self.clear_data)
        layout.addWidget(self.clear_btn)
        
        # Status
        self.status_label = QLabel("Desconectado")
        self.status_label.setStyleSheet("color: red; font-weight: bold;")
        layout.addWidget(self.status_label)
        
        layout.addStretch()
        
        return group
    
    def create_plot_panel(self):
        """Crear panel de gráficos"""
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Configurar PyQtGraph
        pg.setConfigOptions(antialias=True)
        
        # Gráfico FFT principal
        self.fft_plot = pg.PlotWidget(title="FFT Spectrum")
        self.fft_plot.setLabel('left', 'Magnitude')
        self.fft_plot.setLabel('bottom', 'Frequency (Hz)')
        self.fft_plot.showGrid(x=True, y=True)
        layout.addWidget(self.fft_plot)
        
        # Gráfico de comparación de sensores
        self.comparison_plot = pg.PlotWidget(title="Comparación de Sensores")
        self.comparison_plot.setLabel('left', 'Magnitude')
        self.comparison_plot.setLabel('bottom', 'Frequency (Hz)')
        self.comparison_plot.showGrid(x=True, y=True)
        layout.addWidget(self.comparison_plot)
        
        # Controles de visualización
        controls = QGroupBox("Controles de Visualización")
        controls_layout = QGridLayout(controls)
        
        # Checkboxes para sensores
        self.sensor_checks = {}
        colors = ["red", "blue", "green", "orange"]
        sensors = ["ABPLLN", "ELV", "SM4291", "CCDANN600"]
        
        for i, (sensor, color) in enumerate(zip(sensors, colors)):
            check = QCheckBox(sensor)
            check.setChecked(True)
            check.setStyleSheet(f"color: {color}; font-weight: bold;")
            controls_layout.addWidget(check, 0, i)
            self.sensor_checks[sensor] = check
        
        # Rango de frecuencias
        controls_layout.addWidget(QLabel("Freq Min (Hz):"), 1, 0)
        self.freq_min_spin = QSpinBox()
        self.freq_min_spin.setRange(1, 1000)  # Empezar desde 1 Hz para evitar DC
        self.freq_min_spin.setValue(1)  # Valor por defecto 1 Hz en lugar de 0
        controls_layout.addWidget(self.freq_min_spin, 1, 1)
        
        controls_layout.addWidget(QLabel("Freq Max (Hz):"), 1, 2)
        self.freq_max_spin = QSpinBox()
        self.freq_max_spin.setRange(1, 1000)
        self.freq_max_spin.setValue(1000)
        controls_layout.addWidget(self.freq_max_spin, 1, 3)
        
        # Escala logarítmica
        self.log_scale_check = QCheckBox("Escala Logarítmica")
        controls_layout.addWidget(self.log_scale_check, 2, 0, 1, 2)
        
        layout.addWidget(controls)
        
        return widget
    
    def create_log_panel(self):
        """Crear panel de log"""
        group = QGroupBox("Log de Datos")
        layout = QVBoxLayout(group)
        
        self.log_text = QTextEdit()
        self.log_text.setMaximumHeight(300)
        self.log_text.setFont(QFont("Consolas", 9))
        layout.addWidget(self.log_text)
        
        # Información de datos
        info_group = QGroupBox("Información")
        info_layout = QGridLayout(info_group)
        
        self.sensor_info_label = QLabel("Sensor: N/A")
        info_layout.addWidget(self.sensor_info_label, 0, 0)
        
        self.samples_info_label = QLabel("Muestras: 0")
        info_layout.addWidget(self.samples_info_label, 0, 1)
        
        self.freq_res_label = QLabel("Resolución: 0 Hz")
        info_layout.addWidget(self.freq_res_label, 1, 0)
        
        self.peak_freq_label = QLabel("Pico: 0 Hz")
        info_layout.addWidget(self.peak_freq_label, 1, 1)
        
        layout.addWidget(info_group)
        
        return group
    
    def toggle_connection(self):
        """Alternar conexión serie"""
        if not self.serial_reader.running:
            port = self.port_combo.currentText()
            baud = self.baud_spin.value()
            self.serial_reader.set_port(port, baud)
            self.serial_reader.start_reading()
            self.connect_btn.setText("Desconectar")
        else:
            self.serial_reader.stop_reading()
            self.connect_btn.setText("Conectar")
    
    def clear_data(self):
        """Limpiar todos los datos"""
        for sensor in self.sensor_data:
            self.sensor_data[sensor]["fft"] = []
        self.fft_plot.clear()
        self.comparison_plot.clear()
        self.log_text.clear()
    
    def update_status(self, message):
        """Actualizar status"""
        self.status_label.setText(message)
        if "Conectado" in message:
            self.status_label.setStyleSheet("color: green; font-weight: bold;")
        elif "Error" in message:
            self.status_label.setStyleSheet("color: red; font-weight: bold;")
        else:
            self.status_label.setStyleSheet("color: blue; font-weight: bold;")
        
        # Log
        timestamp = time.strftime("%H:%M:%S")
        self.log_text.append(f"[{timestamp}] {message}")
        
    def update_fft_data(self, sensor_name, fft_data):
        """Actualizar datos de FFT"""
        print(f"DEBUG: Recibido {sensor_name} con {len(fft_data)} datos")
        print(f"DEBUG: Sensores disponibles: {list(self.sensor_data.keys())}")
        print(f"DEBUG: Sensor actual antes: {self.current_sensor}")
        
        if sensor_name in self.sensor_data and len(fft_data) > 0:
            self.sensor_data[sensor_name]["fft"] = fft_data.copy()
            self.current_sensor = sensor_name
            
            print(f"DEBUG: Datos guardados para {sensor_name}")
            print(f"DEBUG: Sensor actual después: {self.current_sensor}")
            
            # Actualizar información
            self.sensor_info_label.setText(f"Sensor: {sensor_name}")
            self.samples_info_label.setText(f"Muestras: {len(fft_data)}")
            
            freq_resolution = self.sample_rate / (len(fft_data) * 2)
            self.freq_res_label.setText(f"Resolución: {freq_resolution:.2f} Hz")
            
            # Encontrar frecuencia pico
            if len(fft_data) > 1:
                peak_idx = np.argmax(fft_data[1:]) + 1  # Ignorar DC
                peak_freq = peak_idx * freq_resolution
                self.peak_freq_label.setText(f"Pico: {peak_freq:.2f} Hz")
            
            timestamp = time.strftime("%H:%M:%S")
            self.log_text.append(f"[{timestamp}] FFT recibida de {sensor_name}: {len(fft_data)} bins")
        else:
            print(f"DEBUG: Sensor {sensor_name} no encontrado o datos vacíos")
            print(f"DEBUG: sensor_name in self.sensor_data: {sensor_name in self.sensor_data}")
            print(f"DEBUG: len(fft_data) > 0: {len(fft_data) > 0}")
    
    def update_plots(self):
        """Actualizar gráficos"""
        # Limpiar gráficos
        self.fft_plot.clear()
        self.comparison_plot.clear()
        
        # Debug: verificar datos disponibles
        print(f"DEBUG: Current sensor en update_plots: {self.current_sensor}")
        for sensor, data in self.sensor_data.items():
            if len(data["fft"]) > 0:
                print(f"DEBUG: {sensor} tiene {len(data['fft'])} datos FFT")
        
        # Configurar rango de frecuencias
        freq_min = self.freq_min_spin.value()
        freq_max = self.freq_max_spin.value()
        
        # Configurar escala
        if self.log_scale_check.isChecked():
            self.fft_plot.setLogMode(y=True)
            self.comparison_plot.setLogMode(y=True)
        else:
            self.fft_plot.setLogMode(y=False)
            self.comparison_plot.setLogMode(y=False)
        
        # Plotear sensor actual en gráfico principal
        if self.current_sensor in self.sensor_data:
            fft_data = self.sensor_data[self.current_sensor]["fft"]
            print(f"DEBUG: Ploteando {self.current_sensor} con {len(fft_data)} datos")
            
            if len(fft_data) > 1:  # Necesitamos al menos 2 elementos para ignorar el bin 0
                # Ignorar bin 0 (DC component) - empezar desde índice 1
                frequencies = np.linspace(0, self.sample_rate/2, len(fft_data))[1:]  # Saltar bin 0
                fft_data_no_dc = np.array(fft_data)[1:]  # Saltar bin 0
                
                # Filtrar por rango de frecuencias
                mask = (frequencies >= freq_min) & (frequencies <= freq_max)
                freq_filtered = frequencies[mask]
                fft_filtered = fft_data_no_dc[mask]
                
                print(f"DEBUG: Frecuencias filtradas: {len(freq_filtered)}, FFT filtrada: {len(fft_filtered)}")
                
                color = self.sensor_data[self.current_sensor]["color"]
                self.fft_plot.plot(freq_filtered, fft_filtered, 
                                 pen=pg.mkPen(color, width=2), 
                                 name=self.current_sensor)
        
        # Plotear comparación de sensores
        for sensor_name, sensor_info in self.sensor_data.items():
            if (self.sensor_checks[sensor_name].isChecked() and 
                len(sensor_info["fft"]) > 1):  # Necesitamos al menos 2 elementos
                
                fft_data = sensor_info["fft"]
                # Ignorar bin 0 (DC component) - empezar desde índice 1
                frequencies = np.linspace(0, self.sample_rate/2, len(fft_data))[1:]  # Saltar bin 0
                fft_data_no_dc = np.array(fft_data)[1:]  # Saltar bin 0
                
                # Filtrar por rango de frecuencias
                mask = (frequencies >= freq_min) & (frequencies <= freq_max)
                freq_filtered = frequencies[mask]
                fft_filtered = fft_data_no_dc[mask]
                
                color = sensor_info["color"]
                self.comparison_plot.plot(freq_filtered, fft_filtered, 
                                        pen=pg.mkPen(color, width=2), 
                                        name=sensor_name)

def main():
    """Función principal"""
    app = QApplication(sys.argv)
    
    # Configurar estilo
    app.setStyle('Fusion')
    
    # Crear y mostrar ventana
    window = FFTVisualizer()
    window.show()
    
    sys.exit(app.exec())

if __name__ == "__main__":
    main()