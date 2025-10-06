@echo off
echo ==========================================
echo     FFT Visualizer - Portenta H7
echo ==========================================
echo.

echo Verificando Python...
python --version
if errorlevel 1 (
    echo ERROR: Python no encontrado. Instala Python 3.8 o superior.
    pause
    exit /b 1
)

echo.
echo Instalando dependencias PyQt5...
pip install -r requirements.txt

echo.
echo Iniciando FFT Visualizer...
python fft_visualizer.py

pause