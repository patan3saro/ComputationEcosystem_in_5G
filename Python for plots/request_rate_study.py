import math

import pandas as pd
from matplotlib import pyplot as plt

import utils

work_directory = '/home/saro/Desktop/FirstConfReqRate-06-08-2024'
strategies = ['EC_and_CC', 'VCC_and_CC']
paradigms_all = ['EDGE', 'CLOUD', 'VCC']
column_names = ['id_user', 'size', 'time_initial', 'time_final', 'paradigm',
                'interarrival_time', 'cars_number',
                'users_number', 'uplink_instant', 'elaboration_time', 'edge_resources',
                'vcc_resources', 'queuing_instant', 'workload',
                'used_cars', 'vehicle_queue_length']

EC_5_per_s = [0.00955987, 0.00954269, 0.0095252, 0.01015881, 0.01035865, 0.00991965,
              0.01034862, 0.01012093, 0.00998132, 0.00995325, 0.00995341, 0.00991444,
              0.0100511]  # ,  0.01007172]
VCC_rate_5_per_s = [0.030473195776991552, 0.02662355666273827, 0.02728108951577177, 0.026464849284973665,
                    0.02654086695197214,
                    0.026447311097848147, 0.026456438524097896, 0.026110714119823207, 0.025844007534902784,
                    0.02579638436935544,
                    0.02596692279724464, 0.02586865944575179, 0.025999424928097214]
# 0.027339352405757335]

offload_list_5_per_sec = [EC_5_per_s, VCC_rate_5_per_s]

fontsize = 12
label_font_size = 16

seeds = [0]
users = [1, 2, 5, 10, 15, 20, 25, 30, 40, 50, 60, 80, 100]
x_ax = [str(int(x)) for x in users]
x_pos = [x for x in range(len(users))]
rates = [10]
colors = ('0.05', '0.3', '0.55', '0.8')  # for plotting different styles of lines colors
styles = (':', '-', '--', '-.')  # for plotting different styles of lines styles
markers = ('o', '+', 's', '*')  # for plotting different styles of lines markers

for s in range(len(strategies)):
    for rate in rates:
        y_values = [0] * len(users)
        cloud_usage_array = [0] * len(users)
        for seed in seeds:
            for i in range(len(users)):
                offloading_file = work_directory + '/csv_files/' + strategies[
                    s] + '/speed0' + '/offloading_time_rate_users_' + \
                                  str(rate) + '_' + str(users[i]) + '_' + str(seed) + '.csv'
                total_sent_file = work_directory + '/csv_files/' + strategies[
                    s] + '/speed0' + '/total_sent_packets_rate_users_' + \
                                  str(rate) + '_' + str(users[i]) + '_' + str(seed) + '.csv'

                total_sent_edge = work_directory + '/csv_files/' + strategies[
                    s] + '/speed0' + '/total_EDGE_sent_packets_rate_users_' + \
                                  str(rate) + '_' + str(users[i]) + '_' + str(seed) + '.csv'
                total_sent_cloud = work_directory + '/csv_files/' + strategies[
                    s] + '/speed0' + '/total_CLOUD_sent_packets_rate_users_' + \
                                   str(rate) + '_' + str(users[i]) + '_' + str(seed) + '.csv'

                df = pd.read_csv(offloading_file, skiprows=20, names=column_names)

                # df_total = pd.read_csv(total_sent_file)
                # df_total_EDGE = pd.read_csv(total_sent_edge)
                # df_total_CLOUD = pd.read_csv(total_sent_cloud)
                # Stampa il DataFrame completo

                # computing the usage of cloud or edge/vcc

                # Usare value_counts per contare le occorrenze di ciascun valore
                count_values = df['paradigm'].value_counts()

                # Ottenere i conteggi dei valori specifici
                if s == 0:
                    count_FIRST = count_values.get(0, 0)
                elif s == 1:
                    count_FIRST = count_values.get(2, 0)
                count_CLOUD = count_values.get(1, 0)
                # print(count_CLOUD)

                # valori in percentuale uso del cloud
                if count_CLOUD == 0:
                    perc_usage_CLOUD = 0
                else:
                    perc_usage_CLOUD = count_CLOUD / df['paradigm'].shape[0]
                cloud_usage_array[i] = round(100 * perc_usage_CLOUD)

                df['time_initial'] = df['time_initial'].apply(utils.convert_nanoseconds_to_decimal_seconds)
                df['time_final'] = df['time_final']

                df['offloading_time'] = df['time_final'] - df['time_initial']

                average_offloading = df['offloading_time'].mean()

                y_values[i] = average_offloading

    # Creare il grafico
    # plt.figure(figsize=(8, 6))  # Imposta la dimensione della figura

    # Tracciare la curva
    if s == 0:
        legend = 'ECFirst'
    else:
        legend = 'VCCFirst'

    plt.plot(x_ax, y_values, label=legend + " 100 req/s", marker=markers[s], color=colors[s],
             linestyle=styles[s])  # for plotting different styles of lines colors)
    print(y_values, legend)
    plt.plot(x_ax, offload_list_5_per_sec[s], label=legend + " 5 req/s", marker=markers[s + 2], color=colors[s + 2],
             linestyle=styles[s + 2])
    plt.axhline(y=0.016, color='r', linestyle=':')
    plt.axhline(y=0.1, color='y', linestyle='--')
    plt.axhline(y=0.5, color='g')
    # Personalizzare i tick degli assi
    plt.xticks(x_pos, x_ax, fontsize=fontsize)  # Dimensione dei tick dell'asse x
    plt.yticks(fontsize=fontsize)  # Dimensione dei tick dell'asse y
    plt.yscale('log')

    # Impostare le label degli assi
    plt.xlabel('Users', fontsize=label_font_size)  # Dimensione della label dell'asse x
    plt.ylabel('Offloading time [s]', fontsize=label_font_size)  # Dimensione della label dell'asse y

    # Aggiungere una legenda
    plt.legend(fontsize=fontsize)  # Dimensione del testo della legenda

    print(strategies[s], cloud_usage_array)
    plt.tight_layout()

    plt.draw()

    # Mostrare il grafico

#plt.savefig('/home/saro/Desktop/requests_rate_comparison_users.png', bbox_inches='tight')
plt.show()

# EC_and_CC [0, 0, 0, 0, 0, 38, 50, 63, 70, 72, 76, 83, 86]
# VCC_and_CC [0, 0, 0, 16, 29, 42, 62, 69, 78, 84, 87, 93, 99]
