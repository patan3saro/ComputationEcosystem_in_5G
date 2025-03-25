import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

from utils import convert_nanoseconds_to_decimal_seconds

# Percorso del file CSV
file_path = '/home/saro/NS3/ns-3-dev/scratch/FirstConf/csv_files/RectangleScenario/speed0/VCC_and_CC/offloading_time_workload.csv'

#'/home/

column_names = ['id_user', 'size', 'time_initial', 'time_final', 'paradigm',
                'interarrival_time', 'cars_number',
                'users_number', 'uplink_time', 'elaboration_time', 'edge_resources',
                'vcc_resources',
                'queueing_time', 'workload', 'used_cars', 'vehicle_queue_length']
# Caricamento del CSV in un DataFrame
df = pd.read_csv(file_path, names=column_names)

df['time_int'] = df['time_initial'].apply(convert_nanoseconds_to_decimal_seconds).astype(int)

# Contare il numero di elementi per ciascun secondo
time_counts = df['time_int'].value_counts().sort_index()

# Creazione del grafico
plt.figure(figsize=(10, 6))
plt.plot(time_counts.index, time_counts.values, marker='o', linestyle='-')
plt.axhline(y=30 * 5, color='r', linestyle='--', label='Expected value of requests')

# Aggiunta di etichette e titolo
plt.xlabel('Time (s)')
plt.ylabel('Offloaded requests')
plt.title('Offloaded equests per second')
plt.grid(True)

plt.draw()

# Raggruppiamo i dati per secondi interi e calcoliamo la somma dei veicoli per ciascun secondo
vehicles_per_second = df.groupby('time_int')['used_cars'].mean()

# Visualizzazione dei primi risultati
# print(vehicles_per_second.head(10))  # Mostra i conteggi per i primi 10 secondi

# Creazione del grafico
plt.figure(figsize=(10, 6))
plt.plot(vehicles_per_second.index, vehicles_per_second.values, marker='o', linestyle='-')

# Aggiunta di etichette e titolo
plt.xlabel('Tempo (secondi)')
plt.ylabel('Numero di veicoli disponibili')
plt.title('Numero di veicoli disponibili per secondo')
plt.grid(True)
plt.draw()

# Visualizzazione delle prime righe del conteggio
# print(time_counts)

# Contare il numero di 1 e 2 nella colonna
counts = df['paradigm'].value_counts()

# Visualizzazione dei conteggi
# print(counts)

# Define custom labels
labels = ['VCC', 'CC']

# Create the pie chart
plt.figure(figsize=(8, 6))
counts.plot(kind='pie', labels=labels, autopct='%1.1f%%', startangle=90)

# Add labels and title
plt.ylabel('')  # Removes the y-axis label
plt.title('Distribution of VCC and CC')
plt.draw()

df['time_initial'] = df['time_initial'].apply(convert_nanoseconds_to_decimal_seconds).astype(float)

# Calcoliamo la differenza tra 'column1' e 'column2'
df['offloading_time'] = df['time_final'] - df['time_initial']

# Raggruppiamo i dati per secondi interi e calcoliamo la media della differenza per ciascun secondo
mean_difference_per_second = df.groupby('time_int')['offloading_time'].mean()

# Visualizzazione dei primi risultati
# print(mean_difference_per_second.head(90))  # Mostra i risultati per i primi 10 secondi

# Creazione del grafico
plt.figure(figsize=(10, 6))
plt.plot(mean_difference_per_second.index, mean_difference_per_second.values, marker='o', linestyle='-')

# Aggiunta di etichette e titolo
plt.xlabel('Time (s)')
plt.ylabel('Offloading Time')
plt.title('Offloading time per second')
plt.grid(True)

# Filtriamo le righe dove la differenza è maggiore di 0.1
filtered_df = df[df['offloading_time'] > 0.1]
# Impostazioni per visualizzare l'intero DataFrame
pd.set_option('display.max_rows', None)
pd.set_option('display.max_columns', None)
pd.set_option('display.width', None)
pd.set_option('display.max_colwidth', None)
# Stampa delle righe filtrate
# print(filtered_df)
# Display the chart
plt.draw()

# print the failed requests


# Percorso del file CSV
file_path = '/home/saro/Desktop/FirstConf-16-05-2024/csv_files (copy)/RectangleScenario/05-07-2024-22:18/VCC_and_CC/speed0/total_sent_packets_users_number_30_0.csv'

column_names = ['id_user', 'time_initial']
# Caricamento del CSV in un DataFrame
df2 = pd.read_csv(file_path, names=column_names)

df2['time_int'] = df2['time_initial'].astype(int)

# Contiamo il numero di righe per secondo per ciascun DataFrame
rows_per_second_df = df['time_int'].value_counts().sort_index()
rows_per_second_df2 = df2['time_int'].value_counts().sort_index()

# Calcoliamo la differenza tra i totali di righe per secondo tra i due DataFrame
difference_per_second = rows_per_second_df2 - rows_per_second_df

# Creiamo un grafico per visualizzare la differenza tra i totali di righe per secondo
plt.figure(figsize=(10, 6))
plt.plot(difference_per_second.index, difference_per_second.values, label='Failure rate', color='red')
plt.xlabel('Time (s)')
plt.ylabel('Failure rate')
plt.title('Failure rate per second')
plt.legend()
plt.grid(True)
plt.draw()
# Contare il numero di elementi per ciascun secondo
time_counts = df2['time_int'].value_counts().sort_index()

# Creazione del grafico
plt.figure(figsize=(10, 6))
plt.plot(time_counts.index, time_counts.values, marker='o', linestyle='-')
plt.axhline(y=30 * 5, color='r', linestyle='--', label='Expected value of requests')

# Aggiunta di etichette e titolo
plt.xlabel('Time (s)')
plt.ylabel('Requests')
plt.title('Requests per second')
plt.grid(True)
plt.draw()

# where the failure

file_path_edge_received = '/home/saro/Desktop/FirstConf-16-05-2024/csv_files (copy)/RectangleScenario/05-07-2024-22:18/VCC_and_CC/speed0/total_VCC_sent_packets_users_number_30_0.csv'

file_path_VCC_received = '/home/saro/Desktop/FirstConf-16-05-2024/csv_files (copy)/RectangleScenario/05-07-2024-22:18/VCC_and_CC/speed0/total_VCC_received_from_edge_packets_users_number_30_0.csv'

column_names = ['id_user', 'size', 'time_initial', 'time_final', 'paradigm',
                'interarrival_time', 'cars_number',
                'users_number', 'edge_resources',
                'vcc_resources', 'workload', 'used_cars', 'vehicle_queue_length']

df3 = pd.read_csv(file_path_edge_received, names=column_names)
df4 = pd.read_csv(file_path_VCC_received, names=column_names)

df3['time_int'] = df3['time_initial'].apply(convert_nanoseconds_to_decimal_seconds).astype(int)
df4['time_int'] = df4['time_initial'].apply(convert_nanoseconds_to_decimal_seconds).astype(int)

rows_per_second_df3 = df3['time_int'].value_counts().sort_index()
rows_per_second_df4 = df4['time_int'].value_counts().sort_index()

# Calcoliamo la differenza tra i totali di righe per secondo tra i due DataFrame
difference_per_second_usr_gNB = rows_per_second_df2 - rows_per_second_df3
difference_per_second_gNB_VCC = rows_per_second_df3 - rows_per_second_df4

difference_per_second_VCC_usr = rows_per_second_df4 - rows_per_second_df

# Creiamo un grafico per visualizzare la differenza tra i totali di righe per secondo
plt.figure(figsize=(10, 6))
plt.plot(difference_per_second_usr_gNB.index, difference_per_second_usr_gNB.values, label='Failure rate usr-gnb',
         color='red')
plt.plot(difference_per_second_gNB_VCC.index, difference_per_second_usr_gNB.values, label='Failure rate gnb-vcc',
         color='b')
plt.plot(difference_per_second_VCC_usr.index, difference_per_second_VCC_usr.values, label='Failure rate vcc-usr',
         color='y')
plt.xlabel('Time (s)')
plt.ylabel('Failure rate usr-gNB')
plt.title('Failure rate per second usr to gNB')
plt.legend()
plt.grid(True)
plt.draw()

# requests time

# Se i tempi sono in secondi, convertili in millisecondi
df['tempi_partenza_ms'] = df['time_final'] * 1000

# Raggruppa i tempi in intervalli di 100 millisecondi
df['bins'] = pd.cut(df['tempi_partenza_ms'], bins=np.arange(0, df['tempi_partenza_ms'].max() + 100, 100))

# Conta il numero di richieste in ciascun intervallo
request_counts = df['bins'].value_counts().sort_index()

# Estrai i centri degli intervalli per posizionare i pallini
interval_centers = [interval.left + 50 for interval in request_counts.index.categories]

# Crea il grafico
plt.figure(figsize=(10, 5))
plt.scatter(interval_centers, request_counts, color='blue', s=50)  # s è la dimensione dei pallini
plt.xlabel('Tempo (millisecondi)')
plt.ylabel('Numero di richieste')
plt.title('Richieste ogni 100 millisecondi')
plt.grid(True)
plt.draw()


# Specifica il percorso del file
file_path = '/home/saro/NS3/ns-3-dev/scratch/FirstConf/csv_files/RectangleScenario/speed0/VCC_and_CC/0/total_VCC_beaconsworkload.csv'

# Leggere il file CSV
data = pd.read_csv(file_path)

# Assicurati che la colonna dei secondi sia interpretata come numerica
data.iloc[:, 3] = pd.to_numeric(data.iloc[:, 3], errors='coerce')

# Creare una nuova colonna per raggruppare in intervalli di un secondo
data['Time Interval'] = data.iloc[:, 3].round()

# Raggruppare i dati per intervallo di tempo e contare il numero di righe per ogni intervallo
line_counts = data.groupby('Time Interval').size()

# Stampare il numero totale di elementi
print("Numero totale di elementi:", line_counts.sum())

# Creare il grafico
plt.figure(figsize=(10, 6))
plt.plot(line_counts.index, line_counts.values, marker='o')
plt.xlabel('Intervallo di Tempo (secondi)')
plt.ylabel('Numero di Righe')
plt.title('Numero di Righe per Intervallo di Secondo')
plt.grid(True)
plt.show()


plt.show()



