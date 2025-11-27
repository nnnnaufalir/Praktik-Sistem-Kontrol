import serial
import serial.tools.list_ports
import time
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button # Import Widget Tombol
from datetime import datetime

# --- 1. SETUP PORT OTOMATIS ---
def get_serial_port():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("Tidak ada port serial yang terdeteksi!")
        return None
    for i, p in enumerate(ports):
        print(f"[{i}] {p.device} - {p.description}")
    while True:
        try:
            choice = int(input("Pilih nomor port (contoh: 0): "))
            if 0 <= choice < len(ports):
                return ports[choice].device
        except ValueError:
            pass

selected_port = get_serial_port()
if not selected_port: exit()

BAUD_RATE = 115200
waktu_start = datetime.now().strftime("%Y%m%d_%H%M%S")
EXCEL_FILENAME = f'Data_Log_{waktu_start}.xlsx'

# --- INISIALISASI SERIAL ---
try:
    ser = serial.Serial(selected_port, BAUD_RATE, timeout=1)
    print(f"Terhubung ke {selected_port}...")
    time.sleep(0.1)
except Exception as e:
    print(f"Error: {e}")
    exit()

# --- VARIABEL GLOBAL ---
data_history = {'Waktu': []}
lines_dict = {}
is_running = True # Status logging

# --- SETUP PLOT & TOMBOL ---
plt.style.use('bmh')
# Kita atur margin bawah (bottom=0.2) agar ada ruang untuk tombol
fig, ax = plt.subplots()
plt.subplots_adjust(bottom=0.2) 

ax.set_title(f"Universal Logger ({selected_port})")
ax.set_xlabel("Data Points")
ax.set_ylabel("Nilai")
colors = ['blue', 'red', 'green', 'orange', 'purple', 'brown']

# Fungsi Simpan Data
def save_data(event=None):
    global is_running
    is_running = False # Hentikan update grafik
    
    if ser.is_open: ser.close()
    
    # Hentikan animasi
    ani.event_source.stop()
    
    print("\nLogging Dihentikan User.")
    if len(data_history['Waktu']) > 0:
        print(f"Menyimpan data ke {EXCEL_FILENAME}...")
        df = pd.DataFrame(data_history)
        df.to_excel(EXCEL_FILENAME, index=False)
        print("Selesai! Anda boleh menutup jendela grafik.")
        ax.set_title(f"LOGGING SELESAI - Data Tersimpan")
    else:
        print("Tidak ada data tersimpan.")

# Buat Tombol di Grafik
# Posisi [kiri, bawah, lebar, tinggi]
ax_btn = plt.axes([0.7, 0.05, 0.2, 0.075]) 
btn = Button(ax_btn, 'STOP & SAVE', color='red', hovercolor='pink')
btn.on_clicked(save_data) # Hubungkan tombol dengan fungsi save_data

def update_plot(frame):
    if not is_running: return # Stop update jika tombol sudah ditekan
    
    try:
        if ser.in_waiting:
            raw_line = ser.readline().decode('utf-8', errors='ignore').strip()
            if ':' not in raw_line: return 

            timestamp = datetime.now().strftime('%H:%M:%S')
            parts = raw_line.split(',')
            current_vals = {'Waktu': timestamp}
            
            for part in parts:
                if ':' in part:
                    label, val = part.split(':')
                    try:
                        current_vals[label.strip()] = float(val)
                    except: continue

            # Update Dictionary
            data_history['Waktu'].append(timestamp)
            for key, val in current_vals.items():
                if key == 'Waktu': continue
                
                if key not in data_history:
                    # Logic buat key/garis baru
                    data_history[key] = [None] * (len(data_history['Waktu']) - 1)
                    data_history[key].append(val)
                    color = colors[len(lines_dict) % len(colors)]
                    line, = ax.plot([], [], label=key, color=color)
                    lines_dict[key] = line
                    ax.legend(loc='upper left')
                else:
                    data_history[key].append(val)

            # Sinkronisasi Data
            max_len = len(data_history['Waktu'])
            for key in data_history:
                if len(data_history[key]) < max_len:
                    last = data_history[key][-1] if data_history[key] else 0
                    data_history[key].append(last)

            # Update Visual
            x_idx = range(max_len)
            for key, line in lines_dict.items():
                line.set_data(x_idx, data_history[key])

            if max_len > ax.get_xlim()[1]:
                ax.set_xlim(0, max_len + 20)
            ax.relim()
            ax.autoscale_view(scalex=False, scaley=True)

            print(f"[{timestamp}] Data: {current_vals}")

    except Exception: pass

ani = FuncAnimation(fig, update_plot, interval=1)

print("\nScanning... Tekan tombol 'STOP & SAVE' di jendela grafik untuk berhenti.")
plt.show()