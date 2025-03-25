import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import re

import seaborn as sns

# Leggi il file TCL e carica i dati in un DataFrame pandas
file_path = '/home/saro/Desktop/FirstConf-27-05-2024/sumo/RectangleScenario/speed0/Krauss/40/manhattan_Krauss_40_0.tcl'

import pandas as pd
import re

# Leggi il file in un dataframe
with open(file_path, 'r') as file:
    lines = file.readlines()

# Definisci il modello di riga che desideri trovare usando regex
pattern = r'\$ns_ at (\d+\.\d+) "\$node_\((\d+)\) setdest (-?\d+\.\d+) (-?\d+\.\d+) (-?\d+\.\d+)"'

# Inizializza le liste per i valori delle colonne
at_values = []
node_values = []
setdest_values = []

# Filtra le righe che corrispondono al modello
for line in lines:
    match = re.match(pattern, line.strip())
    if match:
        at_value = float(match.group(1))
        node_value = int(match.group(2))
        setdest_value = float(match.group(5))
        at_values.append(at_value)
        node_values.append(node_value)
        setdest_values.append(setdest_value)

# Crea un dataframe con i valori estratti
df = pd.DataFrame({
    'At': at_values,
    'Node': node_values,
    'Setdest': setdest_values
})

# Raggruppa i valori in base al nodo e calcola la distribuzione
distribution_per_node = df.groupby('Node')['Setdest']


# Calcola i ticks sull'asse y con un incremento di 0.1
y_ticks = np.arange(df['Setdest'].min(), df['Setdest'].max() + 0.1, 0.1)

km_per_hour = 13.1
m_per_second = km_per_hour * 1000 / 3600

plt.figure(figsize=(12, 8))
plt.axhline(y=m_per_second, color='red', linestyle='--', label=str(m_per_second)+' km/h')
sns.scatterplot(data=df, x='Node', y='Setdest', alpha=0.5)
plt.xticks(df['Node'].unique())  # Imposta i ticks sull'asse x per corrispondere a tutti i valori unici dei nodi
plt.yticks(y_ticks)  # Imposta i ticks sull'asse y con incremento di 0.1
plt.xlabel('Node')
plt.ylabel('Speed [m/s]')
plt.grid(True)
plt.show()
