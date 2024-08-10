import tkinter as tk
import threading
import serial
import math
from tkinter import simpledialog

class GridApp:
    def __init__(self, root, rows=10, columns=10, agent_position=None, agent_color="red", agent_size=(20, 40), agent_angle=0, update_interval=50):
        self.root = root
        self.root.title("Grid App")
        self.rows = rows
        self.columns = columns
        self.cell_size = 50  # Tamaño de cada celda en píxeles
        self.update_interval = update_interval  # Intervalo de actualización en milisegundos

        if agent_position is None:
            self.agent_position = (50, 50)
        else:
            self.agent_position = agent_position

        self.agent_color = agent_color
        self.agent_size = agent_size
        self.agent_angle = agent_angle

        self.current_position = list(self.agent_position)
        self.current_angle = self.agent_angle

        self.prev_encoder_values = [0, 0, 0]
        self.trajectory_coords = [self.current_position[0], self.current_position[1], self.current_position[0], self.current_position[1]]

        self.create_menu()
        self.create_canvas()
        self.draw_grid()
        self.create_agent()
        self.create_trajectory()

        self.update_needed = False

        self.root.after(self.update_interval, self.periodic_update)

    def create_menu(self):
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)

        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Change Grid Size", command=self.change_grid_size)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit)

    def create_canvas(self):
        canvas_width = self.columns * self.cell_size
        canvas_height = self.rows * self.cell_size
        self.canvas = tk.Canvas(self.root, width=canvas_width, height=canvas_height, bg="white")
        self.canvas.pack(padx=20, pady=20)

    def draw_grid(self):
        for r in range(self.rows):
            for c in range(self.columns):
                x1 = c * self.cell_size
                y1 = r * self.cell_size
                x2 = x1 + self.cell_size
                y2 = y1 + self.cell_size
                self.canvas.create_rectangle(x1, y1, x2, y2, outline="black")

    def create_agent(self):
        x, y = self.current_position  # Asegurarse de usar la posición actual
        self.agent = self.draw_rotated_rectangle(x, y, self.agent_size, self.current_angle, self.agent_color)

    def draw_rotated_rectangle(self, cx, cy, size, angle, color):
        w, h = size
        angle = math.radians(angle)

        # Calcular las coordenadas de las esquinas del rectángulo
        dx = w / 2
        dy = h / 2

        corners = [
            (-dx, -dy),
            (dx, -dy),
            (dx, dy),
            (-dx, dy)
        ]

        rotated_corners = [
            (cx + x * math.cos(angle) - y * math.sin(angle), cy + x * math.sin(angle) + y * math.cos(angle))
            for x, y in corners
        ]

        return self.canvas.create_polygon(rotated_corners, fill=color, outline=color)

    def create_trajectory(self):
        self.trajectory = self.canvas.create_line(self.trajectory_coords, fill="blue", width=3)

    def update_trajectory(self, new_x, new_y):
        self.trajectory_coords.extend([new_x, new_y])
        self.canvas.coords(self.trajectory, *self.trajectory_coords)

    def move_agent(self, dx, dy, dangle=None):
        if dangle is not None:
            self.current_angle -= dangle  # Invertir el signo del ángulo
        # Transformar dx y dy según el ángulo actual del robot
        angle_rad = math.radians(self.current_angle)
        transformed_dx = dx * math.cos(angle_rad) - dy * math.sin(angle_rad)
        transformed_dy = dx * math.sin(angle_rad) + dy * math.cos(angle_rad)

        new_position = [
            self.current_position[0] + transformed_dy,
            self.current_position[1] - transformed_dx
        ]

        # Actualizar la posición actual del agente
        self.current_position = new_position

        # Marcar que se necesita una actualización
        self.update_needed = True

    def periodic_update(self):
        if self.update_needed:
            # Dibujar la trayectoria del agente desde el centro del agente
            self.update_trajectory(self.current_position[0], self.current_position[1])

            # Actualizar la posición del agente
            self.canvas.delete(self.agent)
            self.agent = self.draw_rotated_rectangle(self.current_position[0], self.current_position[1], self.agent_size, self.current_angle, self.agent_color)

            # Resetear la bandera de actualización
            self.update_needed = False

        # Programar la próxima actualización
        self.root.after(self.update_interval, self.periodic_update)

    def change_grid_size(self):
        new_size = simpledialog.askinteger("Input", "Enter new grid size (N x N):", minvalue=1, maxvalue=50)
        if new_size:
            self.rows = new_size
            self.columns = new_size
            self.canvas.config(width=self.columns * self.cell_size, height=self.rows * self.cell_size)
            self.canvas.delete("all")
            self.draw_grid()
            self.create_agent()
            self.create_trajectory()

def pulse_to_cm(x):
    radio = 2.5  # Radio de la rueda 2.5cm
    return ((2 * math.pi * radio) / 600) * x

def odometry(prev_distances, curr_distances):
    L = 25  # Separacion de ruedas laterales
    a_prev, b_prev, c_prev = prev_distances
    a_curr, b_curr, c_curr = curr_distances

    # Calcular las diferencias en las distancias
    delta_a = a_curr - a_prev
    delta_b = b_curr - b_prev
    delta_c = c_curr - c_prev

    # Desplazamiento lineal promedio
    d = (delta_a + delta_c) / 2

    # Desplazamiento angular
    theta = (delta_c - delta_a) / L

    # Cambio en las coordenadas x y y
    delta_x = d - delta_b * theta
    delta_y = delta_b

    return delta_x, delta_y, theta

def update_agent_from_serial(app, ser):
    while True:
        try:
            if ser.in_waiting > 0:
                data = ser.readline().decode().strip()
                if data:
                    # Separar los datos en las variables a, b, c
                    a, b, c = map(float, data.split(','))
                    print(f"a = {a}, b = {b}, c = {c}")
                    curr_encoder_values = [a, b, c]
                    prev_distances_cm = [pulse_to_cm(x) for x in app.prev_encoder_values]
                    curr_distances_cm = [pulse_to_cm(x) for x in curr_encoder_values]
                    dx, dy, dtheta = odometry(prev_distances_cm, curr_distances_cm)

                    # Mover el agente a las nuevas posiciones y ángulo solo si hay un cambio significativo
                    if dx != 0 or dy != 0 or dtheta != 0:
                        app.move_agent(dx, dy, math.degrees(dtheta))

                    # Actualizar los valores previos del encoder
                    app.prev_encoder_values = curr_encoder_values
        except ValueError:
            print("Error en los datos recibidos")
        except KeyboardInterrupt:
            print("Interrupción por el usuario")
            break

if __name__ == "__main__":
    root = tk.Tk()
    agent_position = (50, 50)
    agent_color = "red"
    agent_size = (20, 40)  # Width, Height (ancho, alto)
    agent_angle = 0  # Angle in degrees (0 significa vertical)

    app = GridApp(root, agent_position=agent_position, agent_color=agent_color, agent_size=agent_size, agent_angle=agent_angle)

    # Configurar la conexión serial
    ser = serial.Serial('/dev/ttyUSB3', 9600, timeout=1)

    # Ejecutar el bucle de la GUI en un hilo separado
    threading.Thread(target=update_agent_from_serial, args=(app, ser), daemon=True).start()

    root.mainloop()

    ser.close()  # No olvides cerrar la conexión serial
