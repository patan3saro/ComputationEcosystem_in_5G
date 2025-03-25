import numpy as np
import matplotlib.pyplot as plt

# Generazione dei dati
np.random.seed(42)  # Per riproducibilità
x = np.arange(1, 11)
y = np.random.randint(1, 100, 10)

# Calcolo dei percentili
percentile_95 = np.percentile(y, 95)
percentile_90 = np.percentile(y, 90)
percentile_99 = np.percentile(y, 99)

# Creazione del grafico a dispersione
plt.scatter(x, y, color='black', label='Valori')

# Aggiunta delle linee orizzontali per i percentili
plt.axhline(percentile_95, color='r', linestyle='dashed', linewidth=2, label='95° Percentile')
plt.axhline(percentile_90, color='g', linestyle='dashed', linewidth=2, label='90° Percentile')
plt.axhline(percentile_99, color='b', linestyle='dashed', linewidth=2, label='99° Percentile')

# Aggiunta delle etichette e del titolo
plt.title('Distribuzione dei valori con percentili')
plt.xlabel('X (da 1 a 10)')
plt.ylabel('Y (valori aleatori)')
plt.legend()

# Mostrare il grafico
plt.show()
