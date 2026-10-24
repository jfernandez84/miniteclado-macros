#!/usr/bin/env python3
"""Probador de teclas para el miniteclado de macros.

Muestra cada tecla que llega al Mac (nombre, código y milisegundos desde la
anterior), reconstruye el texto escrito y cuenta cuántas veces se pulsó Delete.
Sirve para comprobar Delete, F10, Esc, las flechas, "7", fs1:, startup.nsh, etc.

Uso:  python3 probador_teclas.py
Deja la ventana en primer plano y pulsa los botones del miniteclado.

Nota para F10: si macOS lo toma como tecla multimedia (silenciar) en lugar de
F10, activa Ajustes del Sistema > Teclado > Atajos de teclado > Teclas de
función > "Usar teclas F1, F2, etc. como teclas de función estándar".
"""
import time
import tkinter as tk
from tkinter import font as tkfont

ESPECIALES = {
    "Return": "↵",
    "KP_Enter": "↵",
    "BackSpace": "⌫",
    "Delete": "⌦",
    "Escape": "⎋",
    "Tab": "⇥",
    "space": " ",
    "Up": "↑",
    "Down": "↓",
    "Left": "←",
    "Right": "→",
}
MODIFICADORES = {"Shift_L", "Shift_R", "Control_L", "Control_R", "Alt_L", "Alt_R",
                 "Meta_L", "Meta_R", "Caps_Lock", "Super_L", "Super_R"}


class Probador:
    def __init__(self, raiz):
        self.raiz = raiz
        raiz.title("Probador de teclas — miniteclado")
        raiz.geometry("720x560")

        mono = tkfont.Font(family="Menlo", size=12)
        grande = tkfont.Font(family="Menlo", size=18, weight="bold")

        self.ultima = None
        self.total = 0
        self.dels = 0
        self.primer_del = None
        self.ultimo_del = None
        self.texto = ""

        marco = tk.Frame(raiz, padx=12, pady=10)
        marco.pack(fill="both", expand=True)

        tk.Label(marco, text="Última tecla", anchor="w").pack(fill="x")
        self.tecla_var = tk.StringVar(value="—")
        tk.Label(marco, textvariable=self.tecla_var, font=grande, anchor="w").pack(fill="x")

        tk.Label(marco, text="Texto reconstruido (↵ = Enter, ⌦ = Delete, ⎋ = Esc)",
                 anchor="w").pack(fill="x", pady=(10, 0))
        self.texto_var = tk.StringVar(value="")
        tk.Label(marco, textvariable=self.texto_var, font=mono, anchor="w",
                 justify="left", wraplength=680, relief="sunken", bg="white",
                 fg="black", padx=6, pady=6).pack(fill="x")

        self.resumen_var = tk.StringVar(value="Teclas: 0   Delete: 0")
        tk.Label(marco, textvariable=self.resumen_var, anchor="w").pack(fill="x", pady=(10, 0))

        tk.Label(marco, text="Registro (tecla · código · ms desde la anterior)",
                 anchor="w").pack(fill="x", pady=(10, 0))
        zona = tk.Frame(marco)
        zona.pack(fill="both", expand=True)
        barra = tk.Scrollbar(zona)
        barra.pack(side="right", fill="y")
        self.log = tk.Text(zona, font=mono, height=12, yscrollcommand=barra.set,
                           state="disabled")
        self.log.pack(side="left", fill="both", expand=True)
        barra.config(command=self.log.yview)

        tk.Button(marco, text="Limpiar", command=self.limpiar).pack(anchor="e", pady=(8, 0))

        raiz.bind_all("<KeyPress>", self.al_pulsar)
        raiz.focus_force()

    def limpiar(self):
        self.ultima = None
        self.total = 0
        self.dels = 0
        self.primer_del = None
        self.ultimo_del = None
        self.texto = ""
        self.texto_var.set("")
        self.tecla_var.set("—")
        self.resumen_var.set("Teclas: 0   Delete: 0")
        self.log.config(state="normal")
        self.log.delete("1.0", "end")
        self.log.config(state="disabled")

    def al_pulsar(self, ev):
        ahora = time.perf_counter()
        nombre = ev.keysym
        if nombre in MODIFICADORES:
            return

        delta = "" if self.ultima is None else f"{(ahora - self.ultima) * 1000:7.1f} ms"
        self.ultima = ahora
        self.total += 1

        if nombre == "Delete":
            self.dels += 1
            if self.primer_del is None:
                self.primer_del = ahora
            self.ultimo_del = ahora

        simbolo = ESPECIALES.get(nombre) or (ev.char if ev.char and ev.char.isprintable() else f"<{nombre}>")
        self.texto += simbolo if nombre != "Return" else "↵"
        self.texto_var.set(self.texto[-400:])
        self.tecla_var.set(f"{nombre}   (código {ev.keycode})")

        linea = f"{nombre:<12} {ev.keycode:>6}  {delta}\n"
        self.log.config(state="normal")
        self.log.insert("end", linea)
        self.log.see("end")
        self.log.config(state="disabled")

        resumen = f"Teclas: {self.total}   Delete: {self.dels}"
        if self.dels > 1 and self.ultimo_del > self.primer_del:
            seg = self.ultimo_del - self.primer_del
            resumen += f"   ({self.dels / seg:.1f} Delete/s durante {seg:.1f} s)"
        self.resumen_var.set(resumen)


if __name__ == "__main__":
    raiz = tk.Tk()
    Probador(raiz)
    raiz.mainloop()
