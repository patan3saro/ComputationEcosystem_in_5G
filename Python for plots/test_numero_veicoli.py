import numpy as np

# Parametri
num_veicoli = 40
richieste_per_veicolo = 8
tempo_arrivo_richieste = 0.2 # in secondi
tempo_calcolo_per_richiesta = 0.02 # in secondi (caso peggiore)
tempo_messaggio = 100 # in secondi
tempo_totale = 120 # in secondi

# Tempo per processare tutte le richieste per veicolo
tempo_processamento = richieste_per_veicolo * tempo_calcolo_per_richiesta

# Numero di iterazioni nel tempo totale
num_iterazioni = int(tempo_totale / tempo_arrivo_richieste)

# Array per tenere traccia del numero di veicoli disponibili nel tempo
veicoli_disponibili = np.zeros(num_iterazioni)

# Inizialmente tutti i veicoli sono disponibili
veicoli_disponibili[:] = num_veicoli

# Simulazione
for t in range(num_iterazioni):
    current_time = t * tempo_arrivo_richieste

    # Verifica se i veicoli sono disponibili
    if current_time >= tempo_messaggio:
        veicoli_disponibili[t] = num_veicoli - int(current_time / tempo_messaggio)
    else:
        veicoli_disponibili[t] = num_veicoli

# Genera l'asse dei tempi
tempi = np.linspace(0, tempo_totale, num_iterazioni)

# Stampa la successione dei valori
for i in range(num_iterazioni):
    print(f"Tempo: {tempi[i]:.2f} secondi - Veicoli Disponibili: {veicoli_disponibili[i]}")
