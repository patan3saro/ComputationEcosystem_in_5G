""" This file generates automatically all the results """
import csv
import decimal
import matplotlib.pyplot as plt
from pathlib import Path
import numpy as np
from utils import failure_rate_for_any_config, convert_nanoseconds_to_decimal_seconds, \
    cars_number_in_vcc_for_any_config, plot_paradigm_for_any_config, workload_strategy_comparison, \
    users_strategy_comparison, dwell_time_per_speed_per_vehicle_num, \
    plot_uplink_distr  # ,graph_vehicles_used_over_time

work_directory = '/home/saro/Desktop/FirstConf-19-06-2024'
run_instants = ['06-13-2024-22:59']
strategies = ['EC_and_CC', 'VCC_and_CC']
paradigms_all = ['EDGE', 'CLOUD', 'VCC']
# sumo_scenarios = ('RectangleScenario', 'GridScenario')
sumo_scenarios = ['RectangleScenario']
vehicles_num = 40
simulation_duration = 120  # seconds
x_axs = [0] * 2
x_axs[0] = (100, 200, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 6000, 8000, 10000)
# x_axs[1] = (1, 2, 5, 10, 15, 20, 25, 30)
x_axs[1] = (1, 2, 5, 10, 15, 20, 25, 30, 40, 50, 60, 80, 100, 150)

workload_comparison_offloading_time = [0] * 2
users_comparison_offloading_time = [0] * 2

dwell_time_files = []
piechart_for_users_number_offloading_time_division_data = np.array([])



for strategy in strategies:

    # useful for all the strategies
    users_number = {
        "type": "users_number",
        # "x_axis": (1, 2, 5, 10, 15, 20, 25, 30),
        "x_axis": (1, 2, 5, 10, 15, 20, 25, 30, 40, 50, 60, 80, 100, 150),
        "index": 7,
        "index_failure": 7,
        "x_label": "Users"
    }

    workload = {
        "type": "workload",
        "x_axis": (100, 200, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 6000, 8000, 10000),
        "index": 13,
        "index_failure": 10,
        "x_label": "Workload [MI]"
    }

    if strategy == strategies[0]:

        paradigms = ('EDGE', 'CLOUD')

        edge_resources = {
            "type": "edge_resources",
            "x_axis": (74907, 149814, 224721, 374535, 599256, 749070, 1498140, 2247210),
            "index": 10,
            "index_failure": 8,
            "x_label": "Edge resources [MIPS]"
        }

        simulation_types = (users_number, workload, edge_resources)
        speeds = ['/speed0']

    elif strategy == strategies[1]:
        paradigms = ('CLOUD', 'VCC')

        cars_number = {
            "type": "cars_number",
            "x_axis": (10, 20, 40, 60),
            #"x_axis": (1, 2, 3, 4, 5, 10, 20, 40, 60),
            "index": 6,
            "index_failure": 6,
            "x_label": "Vehicles"
        }
        carComputationCapacity = 71120

        VCC_resources = (555.625, 1111.25, 2222.5, 4445.0, 8890.0, 35560.0, 71120, 142240, 213360)

        vcc_resources = {
            "type": "vcc_resources",
            "x_axis": VCC_resources,
            "index": 11,
            "index_failure": 9,
            "x_label": "Per vehicle resources [MIPS]"
        }
        file_used_cars_number = np.zeros((len(cars_number["x_axis"]), 10), dtype='object')
        simulation_types = (users_number, workload, cars_number, vcc_resources)
        speeds = ('/speed0', '/speed1', '/speed2')

    for sumo_scenario in sumo_scenarios:
        for simulation in simulation_types:
            sim_type = simulation["type"]
            x_label = simulation["x_label"]
            x_axis = simulation["x_axis"]
            index = simulation["index"]
            index_failure = simulation["index_failure"]
            for speed in speeds:
                input_file_names = []
                cars_number_files = []
                # to calculate the failure rate for any paradigm
                input_total_VCC_sent_packets_files = []
                input_total_EDGE_sent_packets_files = []
                input_total_CLOUD_sent_packets_files = []
                input_from_edge_to_VCC_packets_files = []

                fig, ax1 = plt.subplots()
                all_offloading_per_all_seeds = [[] for item in range(len(x_axis))]
                for run_instant in run_instants:
                    if run_instant == run_instants[0]:
                        runs = [0, 1, 2, 3, 4, 6, 7, 8, 9]

                    for r in runs:
                        for value in simulation["x_axis"]:
                            csv_file = work_directory + '/csv_files/' + sumo_scenario + '/' + run_instant + '/' + strategy + speed + '/offloading_time_' + \
                                       simulation["type"] + '_' + str(value) + '_' + str(r) + '.csv'
                            csv_file_cars = csv_file.replace('/offloading_time_', '/total_VCC_sent_packets_')

                            input_total_VCC_sent_packets_file = csv_file_cars
                            input_total_EDGE_sent_packets_file = csv_file.replace('/offloading_time_',
                                                                                  '/total_EDGE_sent_packets_')
                            input_total_CLOUD_sent_packets_file = csv_file.replace('/offloading_time_',
                                                                                   '/total_CLOUD_sent_packets_')
                            input_from_edge_to_VCC_packets_file = csv_file.replace('/offloading_time_',
                                                                                   '/total_VCC_received_from_edge_packets_')
                            if simulation["type"] == "cars_number" and r == 1:
                                dwell_time_file = csv_file.replace('/offloading_time_', '/dwell_time_')
                                dwell_time_files.append(dwell_time_file)
                            elif simulation["type"] == "cars_number" and speed == '/speed0':
                                file_used_cars_number[int(simulation["x_axis"].index(value))][r] = csv_file

                            input_file_names.append(csv_file)
                            cars_number_files.append(csv_file_cars)

                            input_total_VCC_sent_packets_files.append(input_total_VCC_sent_packets_file)
                            input_total_EDGE_sent_packets_files.append(input_total_EDGE_sent_packets_file)
                            input_total_CLOUD_sent_packets_files.append(input_total_CLOUD_sent_packets_file)
                            input_from_edge_to_VCC_packets_files.append(input_from_edge_to_VCC_packets_file)

                            output_path = work_directory + '/results/' + sumo_scenario + '/' + strategy + speed
                            output_path_create = Path(output_path)
                            output_path_create.mkdir(parents=True, exist_ok=True)
                            output_curve_name = output_path + '/' + simulation["type"]

                for prd in paradigms:
                    choice = paradigms_all.index(prd)
                    len_x = len(simulation["x_axis"])
                    z = np.zeros(len_x)
                    # offloading time
                    y = np.zeros(len_x)
                    # uplink elaboration and dowlink times
                    uplink_times = np.zeros(len_x)
                    queueing_times = np.zeros(len_x)
                    elaboration_times = np.zeros(len_x)
                    downlink_times = np.zeros(len_x)
                    a = [[] for x in range(len_x)]

                    for input_file_name in input_file_names:
                        file = open(input_file_name)

                        csvreader = csv.reader(file)

                        for row in csvreader:

                            paradigm = int(row[4]) == choice

                            offloading_time = float(
                                float(row[3]) - convert_nanoseconds_to_decimal_seconds(row[2]))

                            uplink_time = float(
                                convert_nanoseconds_to_decimal_seconds(
                                    row[8]) - convert_nanoseconds_to_decimal_seconds(row[2]))

                            elaboration_time = float(convert_nanoseconds_to_decimal_seconds(row[9]))
                            queueing_time = float(convert_nanoseconds_to_decimal_seconds(row[12])) - float(
                                convert_nanoseconds_to_decimal_seconds(row[8]))
                            downlink_time = float(
                                float(row[3]) - convert_nanoseconds_to_decimal_seconds(
                                    row[12])) - elaboration_time

                            for k in range(len_x):
                                if "." in row[simulation["index"]]:
                                    temp_var = float(row[simulation["index"]])
                                else:
                                    temp_var = int(float(row[simulation["index"]]))

                                if simulation["x_axis"][k] == temp_var and paradigm:

                                    if strategy == 'VCC_and_CC':
                                        all_offloading_per_all_seeds[k].append(offloading_time)
                                    z[k] += 1
                                    y[k] += offloading_time
                                    a[k].append(offloading_time)
                                    uplink_times[k] += uplink_time
                                    queueing_times[k] += queueing_time
                                    elaboration_times[k] += elaboration_time
                                    downlink_times[k] += downlink_time

                                    if downlink_times[k] < 0:
                                        print(choice,
                                              offloading_time - queueing_time - uplink_time - elaboration_time,
                                              offloading_time,
                                              queueing_time, uplink_time, elaboration_time)
                                        print(row)

                    # to avoid division by zero we replace 0 by 1 in z array
                    z[z == 0] = 1

                    # calculating the average value for each array
                    y /= z
                    uplink_times /= z
                    queueing_times /= z
                    elaboration_times /= z
                    downlink_times /= z

                    # parameters for plots
                    styles = (':', '-', '--')  # for plotting different styles of lines styles
                    markers = ('o', '+', 's')  # for plotting different styles of lines markers
                    colors = ('0.2', '0.5', '0.8')  # for plotting different styles of lines colors
                    fontsize = 12
                    label_font_size = 16
                    max_y = 0

                    x_ax = [str(int(x)) for x in simulation["x_axis"]]
                    x_pos = [x for x in range(len_x)]
                    y_label = 'Offloading time [s]'
                    # note that this plot will overlap the next one
                    if sim_type in ('workload', 'vcc_resources', 'edge_resources', 'users_number'):
                        # ['$C_{VCC}/128$','$C_{VCC}/64$', '$C_{VCC}/32$', '$C_{VCC}/16$', '$C_{VCC}/8$', '$C_{VCC}/4$', '$C_{VCC}/2$', '$C_{VCC}$', '$2C_{VCC}$', '$3C_{VCC}$']
                        plt.xticks(rotation=45)
                    else:
                        # note that this plot will overlap the next one
                        plt.xticks(fontsize=fontsize, rotation=0)
                        plt.yticks(fontsize=fontsize)
                    plt.gcf().subplots_adjust(left=0.2, bottom=0.25)
                    # plt.close()

                    if sim_type == 'workload':
                        if strategy == 'EC_and_CC' and choice == 0:
                            workload_comparison_offloading_time[0] = y

                    elif sim_type == 'users_number':
                        plt.xticks(rotation=45)
                        if strategy == 'EC_and_CC' and choice == 0:
                            users_comparison_offloading_time[0] = y
                        if strategy == 'VCC_and_CC' and choice == 2:
                            width = 0.6
                            fig_insto_perc, ax_insto_perc = plt.subplots()

                            total_times = uplink_times + elaboration_times + downlink_times
                            uplink_times_perc = 100 * (uplink_times / total_times)
                            elaboration_times_perc = 100 * (elaboration_times / total_times)
                            dowlink_times_perc = 100 * (downlink_times / total_times)

                            ax_insto_perc.bar(x_ax, uplink_times_perc, width, label='Uplink')
                            ax_insto_perc.bar(x_ax, elaboration_times_perc, width,
                                              bottom=uplink_times_perc, label='Elaboration')
                            ax_insto_perc.bar(x_ax, dowlink_times_perc, width, label='Downlink',
                                              bottom=uplink_times_perc + elaboration_times_perc)

                            ax_insto_perc.set_xticks(x_pos, x_ax, fontsize=fontsize, rotation=rotate_var)
                            plt.yticks(fontsize=fontsize)
                            ax_insto_perc.set_ylabel("Offloading Time [%]", fontsize=label_font_size)
                            ax_insto_perc.set_xlabel('Users', fontsize=label_font_size)
                            ax_insto_perc.legend(fontsize=fontsize)
                            plt.tight_layout()
                            #fig_insto_perc.savefig(output_curve_name + '_percentage_insto_' + prd + '.png')

                    ax1.set_yscale('log')
                    # now we put the three class or latency low, mid, high
                    ax1.axhline(y=0.016, color='r', linestyle=':')
                    ax1.axhline(y=0.1, color='y', linestyle='--')
                    ax1.axhline(y=0.5, color='g')
                    if sim_type == 'cars_number' and choice == 2:
                        ax1.plot(x_ax, y, linestyle=styles[choice], marker=markers[choice], color=colors[choice],
                                 label='VCCFirst')
                        print("cars number", y)
                        ax1.set_ylim(max(y) + 0.1)
                    elif sim_type == 'cars_number' and choice == 1:
                        pass
                    else:
                        ax1.plot(x_ax, y, linestyle=styles[choice], marker=markers[choice], color=colors[choice],
                                 label=paradigms_all[choice])
                    ax1.set_ylabel(y_label, fontsize=label_font_size, fontfamily='serif')
                    ax1.set_xlabel(x_label, fontsize=label_font_size, fontfamily='serif')
                    ax1.legend(fontsize=fontsize)

                    if sim_type == 'cars_number' and choice == 2:
                        cars_vcc_x = cars_number_in_vcc_for_any_config(cars_number_files,
                                                                       x_axis, index)
                        ax2 = ax1.twiny()
                        ax2.set_xlim(ax1.get_xlim())
                        ax2.set_xlabel("Average used vehicles", fontsize=label_font_size, fontfamily='serif')
                        # ax2.set_ylabel("Average used vehicles", fontsize=label_font_size, fontfamily='serif')
                        # ax2.plot(x_ax, cars_vcc_x, marker='+', color="0.1")

                        x_ax2 = [str(x) for x in cars_vcc_x]
                        ax2.set_xticks(x_pos, x_ax2, fontsize=fontsize, rotation=90)

                    # now we create the instograms for the time division
                    width = 0.6
                    fig_insto, ax_insto = plt.subplots()

                    ax_insto.bar(x_ax, uplink_times, width, label='Uplink')

                    ax_insto.bar(x_ax, queueing_times, width, label='Queueing', bottom=uplink_times)
                    ax_insto.bar(x_ax, elaboration_times, width,
                                 bottom=uplink_times + queueing_times, label='Elaboration')
                    ax_insto.bar(x_ax, downlink_times, width, label='Downlink',
                                 bottom=uplink_times + queueing_times + elaboration_times)
                    rotate_var = 0

                    if sim_type == 'workload':
                        rotate_var = 45
                    ax_insto.set_xticks(x_pos, x_ax, fontsize=fontsize, rotation=rotate_var)
                    plt.yticks(fontsize=fontsize)
                    ax_insto.set_ylabel(y_label, fontsize=label_font_size)
                    ax_insto.set_xlabel(x_label, fontsize=label_font_size)
                    ax_insto.legend(fontsize=fontsize)
                    plt.tight_layout()
                    # new_max = max(y)
                    # if new_max > max_y:
                    #    max_y = new_max
                    #fig_insto.savefig(output_curve_name + '_time_instogram_' + prd + '.png', bbox_inches='tight')

                #fig.savefig(output_curve_name + ".png", bbox_inches='tight')
                plt.clf()
                plt.cla()
                plt.close()
                if strategy == 'VCC_and_CC':
                    # avg 90, 95, 99°
                    avgs = []
                    percentiles_90 = []
                    percentiles_95 = []
                    percentiles_99 = []

                    for lista in all_offloading_per_all_seeds:
                        sorted_offloading_values = np.sort(lista)
                        # Calcola il 9x° percentile
                        percentile_90 = np.percentile(sorted_offloading_values, 90)
                        percentile_95 = np.percentile(sorted_offloading_values, 95)
                        percentile_99 = np.percentile(sorted_offloading_values, 99)
                        percentiles_90.append(percentile_90)
                        percentiles_95.append(percentile_95)
                        percentiles_99.append(percentile_99)
                        average = sum(lista) / len(lista)
                        avgs.append(average)
                    if sim_type == 'workload' and speed == '/speed0':

                        workload_comparison_offloading_time[1] = avgs

                    elif sim_type == "users_number" and speed == '/speed0':

                        users_comparison_offloading_time[1] = avgs

                    fig, ax_total = plt.subplots()
                    ax_total.set_ylabel(y_label, fontsize=label_font_size, fontfamily='serif')
                    ax_total.set_xlabel(x_label, fontsize=label_font_size, fontfamily='serif')
                    ax_total.set_xticks(x_pos, x_ax, fontsize=fontsize)
                    # now we put the three class or latency low, mid, high
                    # Linee orizzontali con etichette
                    ax_total.set_yscale('log')

                    ax_total.axhline(y=0.016, color='r', linestyle=':')
                    # ax_total.text(0.1, 0.016, 'LL++', color='black', va='top', ha='right')

                    ax_total.axhline(y=0.1, color='y', linestyle='--')
                    # ax_total.text(0.09, 0.1, 'LL+', color='black', va='top', ha='right')

                    #ax_total.axhline(y=0.5, color='g')
                    # ax_total.text(0.09, 0.5, 'LL', color='black', va='top', ha='right')

                    if sim_type == "vcc_resources":
                        # Posizioni delle linee verticali
                        vertical_lines = [
                            {"Raspberry Pi 3": 2441},
                            {"Raspberry Pi 4": 7568},
                            {"Raspberry Pi 5": 13231},
                            {"ARM Cortex A73": 71120}
                        ]

                        # Parametri per il testo
                        font_props = {'family': 'serif', 'color': 'black', 'size': 12}

                        # Aggiungi le linee verticali con label
                        for line in vertical_lines:
                            for key, value in line.items():
                                plt.axvline(x=value, color='k', linestyle='--')  # Linea verticale tratteggiata

                                # Calcola la posizione verticale del testo per centrarlo
                                ymin, ymax = plt.gca().get_ylim()
                                y_position = ymin + 0.2 * (ymax - ymin)  # Posizione centrale verticale

                                plt.text(value - 0.5, y_position, key, fontdict=font_props, rotation=90,
                                         va='center')  # Etichetta verticale
                    ax_total.set_yscale('log')

                    if sim_type == 'cars_number' and choice == 2 and speed == '/speed0':
                        cars_vcc_x = cars_number_in_vcc_for_any_config(cars_number_files,
                                                                       x_axis, index)
                        ax_total2 = ax_total.twiny()
                        ax_total2.set_xlim(ax_total.get_xlim())
                        ax_total2.set_xlabel("Average used vehicles", fontsize=label_font_size, fontfamily='serif')

                        x_ax2 = [str(x) for x in cars_vcc_x]
                        ax_total2.set_xticks(x_pos, x_ax2, fontsize=fontsize, rotation=90)

                    ax_total.plot(x_ax, avgs, linestyle=styles[choice], marker='o',
                                  color='0.6', label='Mean')

                    ax_total.plot(x_ax, percentiles_90, linestyle='--', marker='.',
                                  color='0.2', label='90 percentile')
                    ax_total.plot(x_ax, percentiles_95, linestyle=':', marker='x',
                                  color='0.5', label='95 percentile')
                    ax_total.plot(x_ax, percentiles_99, linestyle='-.', marker='*',
                                  color='0.8', label='99 percentile')

                    # ax_total.set_xticks(x_pos, x_ax, fontsize=18, rotation=45)

                    # Ordinare i dati
                    data_sorted = np.sort(avgs)

                    # Calcolare i percentili
                    percentiles = [90, 95, 99]
                    percentile_values = np.percentile(data_sorted, percentiles)

                    # Creare il grafico
                    plt.figure(figsize=(10, 6))
                    plt.plot(data_sorted, label='Data')
                    for n, perc in enumerate(percentiles):
                        plt.axhline(y=percentile_values[n], color='r', linestyle='--', label=f'{perc} percentile')

                    ax_total.set_ylabel(y_label, fontsize=label_font_size, fontfamily='serif')
                    ax_total.set_xlabel(x_label, fontsize=label_font_size, fontfamily='serif')
                    ax_total.legend(fontsize=fontsize)
                    #fig.savefig(output_curve_name + "_total.png", bbox_inches='tight')
                    plt.title('Percentile Curves')
                    # plt.legend()
                    plt.grid(True)
                    plt.clf()
                    plt.cla()
                    plt.close()

                    print(y,x_axis , sim_type)

                satisfied_requests, cloud_perc_requests = plot_paradigm_for_any_config(input_file_names, x_axis, index,
                                                                                       x_label,
                                                                                       output_curve_name)
                # if simulation["type"] == "cars_number" and speed == '/speed0':
                #    print(cloud_perc_requests)

                # failure_rate_for_any_config(input_total_VCC_sent_packets_files, input_total_EDGE_sent_packets_files,
                #                            input_total_CLOUD_sent_packets_files, input_from_edge_to_VCC_packets_files,
                #                            index_failure, x_axis,
                #                            x_label, output_curve_name,
                #                            satisfied_requests)

# graph_vehicles_used_over_time(file_used_cars_number)


dwell_time_per_speed_per_vehicle_num(dwell_time_files, vehicles_num, speeds, work_directory,
                                     simulation_duration,
                                     sumo_scenario)

workload_strategy_comparison(workload_comparison_offloading_time, x_axs[0], workload['x_label'], strategies,
                             work_directory,
                             sumo_scenario)
users_strategy_comparison(users_comparison_offloading_time, x_axs[1], users_number['x_label'], strategies,
                          work_directory,
                          sumo_scenario)
