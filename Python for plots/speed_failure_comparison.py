import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

work_directory = '/home/saro/Desktop/FirstConf-28-06-2024-speed'

speeds = ('speed0', 'speed1', 'speed2')
runs = [0, 1, 3, 4, 5, 7, 8, 9, 10, 11]  # 10 seeds

failure_rate_ped_total_sent_req = np.zeros(3)
failure_BS_received_requests = np.zeros(3)
failure_VCC_received_requests = np.zeros(3)
failure_VCC_sent_to_BS_requests = np.zeros(3)
failure_VCC_rejected = np.zeros(3)
offloaded = np.zeros(3)
failure_BS_ped = np.zeros(3)

for j in range(len(speeds)):
    for r in runs:
        csv_file = f'{work_directory}/csv_files/{speeds[j]}/offloading_time_{speeds[j]}_{r}.csv'

        input_total_VCC_sent_packets_file = csv_file.replace('/offloading_time_', '/total_VCC_sent_packets_')
        input_total_sent_packets_file = csv_file.replace('/offloading_time_', '/total_sent_packets_')
        input_from_edge_to_VCC_packets_file = csv_file.replace('/offloading_time_',
                                                               '/total_VCC_received_from_edge_packets_')
        input_rejected_VCC_packets_file = csv_file.replace('/offloading_time_',
                                                           '/total_VCC_rejected_packets_cause_full_queue_')
        input_BD_to_ped_packets_file = csv_file.replace('/offloading_time_',
                                                        '/response_offloading_passing_in_gNB_')

        # Leggi i dati in un dataframe pandas
        df_ped_total_requests = pd.read_csv(input_total_sent_packets_file)
        df_BS_received_requests = pd.read_csv(input_total_VCC_sent_packets_file)
        df_VCC_received_requests = pd.read_csv(input_from_edge_to_VCC_packets_file)
        df_VCC_sent_to_BS_requests = pd.read_csv(input_BD_to_ped_packets_file)
        df_VCC_sent_to_BS_requests = df_VCC_sent_to_BS_requests[df_VCC_sent_to_BS_requests.iloc[:, 1] == 4028]
        df_rejected = pd.read_csv(input_rejected_VCC_packets_file)
        df_offloaded = pd.read_csv(csv_file)

        # Conta il numero di righe nel dataframe
        failure_rate_ped_total_sent_req[j] += df_ped_total_requests.shape[0]
        failure_BS_received_requests[j] += df_BS_received_requests.shape[0]
        failure_VCC_received_requests[j] += df_VCC_received_requests.shape[0]
        failure_VCC_sent_to_BS_requests[j] += df_VCC_sent_to_BS_requests.shape[0]
        failure_VCC_rejected[j] += df_rejected.shape[0]
        offloaded[j] += df_offloaded.shape[0]

len_runs = len(runs)

# Calcola le medie
failure_rate_ped_total_sent_req = failure_rate_ped_total_sent_req / len_runs
failure_BS_received_requests = failure_BS_received_requests / len_runs
failure_VCC_received_requests = failure_VCC_received_requests / len_runs
failure_VCC_sent_to_BS_requests = failure_VCC_sent_to_BS_requests / len_runs
failure_VCC_rejected = failure_VCC_rejected / len_runs
offloaded = offloaded / len_runs

# Calcola i valori per le barre dell'istogramma
failure_1_2 = 100 * (failure_rate_ped_total_sent_req - failure_BS_received_requests) / failure_rate_ped_total_sent_req
failure_2_3 = 100 * (failure_BS_received_requests - failure_VCC_received_requests) / failure_rate_ped_total_sent_req
failure_3_4 = 100 * (failure_VCC_received_requests - failure_VCC_sent_to_BS_requests) / failure_rate_ped_total_sent_req
failure_4_5 = 100 * (failure_VCC_sent_to_BS_requests - offloaded) / failure_rate_ped_total_sent_req
failure_rejected = 100 * failure_VCC_rejected / failure_rate_ped_total_sent_req

# Creazione dell'istogramma

ind = np.arange(len(speeds))  # Posizione delle barre
width = 0.35  # Larghezza delle barre

fig, ax = plt.subplots()
plt.ylim(0, 5)

bar1 = ax.bar(ind, failure_1_2, width, label='User to gNB (requests)')
bar2 = ax.bar(ind, failure_2_3, width, bottom=failure_1_2, label='gNB to VCC (requests)')
bar3 = ax.bar(ind, failure_rejected, width, bottom=failure_1_2 + failure_2_3, label='gNB to VCC (rejected requests)')
bar4 = ax.bar(ind, failure_3_4, width, bottom=failure_1_2 + failure_2_3 + failure_rejected,
              label='VCC to gNB (response)')
bar5 = ax.bar(ind, failure_4_5, width, bottom=failure_1_2 + failure_2_3 + failure_rejected + failure_3_4,
              label='gNB to User (response)')
fontsize = 12
label_font_size = 16
# Aggiungi etichette e titolo
ax.set_ylabel('Failure rate [%]', fontsize=label_font_size)
ax.set_xlabel('Average speed of vehicles [km/h]', fontsize=label_font_size)
# ax.set_title('Failure rates by speed')
velocities = ('13.1', '50', '100')
ax.set_xticks(ind)
plt.yticks(fontsize=14)
ax.set_xticklabels(velocities, fontsize=fontsize)
ax.legend(loc='upper left', framealpha=0.2)

plt.savefig('/home/saro/Desktop/FirstConf-28-06-2024-speed/speeds.png')

plt.show()
