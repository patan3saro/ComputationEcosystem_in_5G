############## This file generates automatically all the results #############
import csv
import decimal
import matplotlib.pyplot as plt
from pathlib import Path
import numpy as np
from utils import failure_rate_for_any_config, convert_nanoseconds_to_decimal_seconds, \
    cars_number_in_vcc_for_any_config, plot_paradigm_for_any_config, workload_strategy_comparison, \
    users_strategy_comparison, dwell_time_per_speed_per_vehicle_num  # ,graph_vehicles_used_over_time

work_directory = '/home/saro/Desktop/FirstConf-30-04-2024'
run_instants = ('05-07-2024-22:18', '05-09-2024-10:45', '05-10-2024-21:06', '05-12-2024-08:57')
strategies = ('EC_and_CC', 'VCC_and_CC')
paradigms_all = ['EDGE', 'CLOUD', 'VCC']
# sumo_scenarios = ('RectangleScenario', 'GridScenario')
sumo_scenarios = ['RectangleScenario']
vehicles_num = 40
simulation_duration = 120  # seconds
x_axs = [0] * 2
x_axs[0] = (100, 200, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 6000, 8000, 10000)
x_axs[1] = (1, 2, 5, 10, 15, 20, 25, 30)

workload_comparison_offloading_time = [0] * 2
users_comparison_offloading_time = [0] * 2

dwell_time_files = []
piechart_for_users_number_offloading_time_division_data = np.array([])

for strategy in strategies:

    # useful for all the strategies
    users_number = {
        "type": "users_number",
        "x_axis": (1, 2, 5, 10, 15, 20, 25, 30),
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
        speeds = ['']

    elif strategy == strategies[1]:
        paradigms = ('CLOUD', 'VCC')

        cars_number = {
            "type": "cars_number",
            "x_axis": (10, 20, 40, 60),
            "index": 6,
            "index_failure": 6,
            "x_label": "Vehicles"
        }
        edgeComputationCapacity = 749070
        alphas = np.array([0.1, 0.2, 0.4, 0.5, 0.6, 0.8, 1.0])
        VCC_resources = edgeComputationCapacity * alphas
        VCC_Resources = np.insert(VCC_resources, 0, 71120)

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

                for run_instant in run_instants:
                    if run_instant == run_instants[0]:
                        runs = [0, 2, 3]
                    elif run_instant == run_instants[1]:
                        runs = [4]
                    elif run_instant == run_instants[2]:
                        runs = [6, 7]
                    elif run_instant == run_instants[3]:
                        runs = [8, 9]

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
                                decimal.Decimal(row[3]) - convert_nanoseconds_to_decimal_seconds(row[2]))
                            uplink_time = float(
                                convert_nanoseconds_to_decimal_seconds(
                                    row[8]) - convert_nanoseconds_to_decimal_seconds(row[2]))
                            elaboration_time = float(convert_nanoseconds_to_decimal_seconds(row[9]))
                            queueing_time = float(convert_nanoseconds_to_decimal_seconds(row[12])) - float(
                                convert_nanoseconds_to_decimal_seconds(row[8]))
                            downlink_time = float(
                                decimal.Decimal(row[3]) - convert_nanoseconds_to_decimal_seconds(
                                    row[12])) - elaboration_time

                            for k in range(len_x):
                                if simulation["x_axis"][k] == int(float(row[simulation["index"]])) and paradigm:
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
                    fontsize = 26
                    ticks_fontsize = 20
                    max_y = 0

                    x_ax = [str(int(x)) for x in simulation["x_axis"]]
                    x_pos = [x for x in range(len_x)]
                    y_label = 'Offloading time [s]'
                    # note that this plot will overlap the next one
                    if sim_type in ('workload', 'vcc_resources', 'edge_resources'):
                        # ['$C_{VCC}/64$', '$C_{VCC}/32$', '$C_{VCC}/16$', '$C_{VCC}/8$', '$C_{VCC}/4$', '$C_{VCC}/2$', '$C_{VCC}$', '$2C_{VCC}$']
                        plt.xticks(rotation=45)
                    # note that this plot will overlap the next one
                    plt.xticks(fontsize=ticks_fontsize)
                    plt.yticks(fontsize=ticks_fontsize)
                    plt.gcf().subplots_adjust(left=0.2, bottom=0.25)
                    # plt.close()

                    if sim_type == 'workload':
                        if strategy == 'EC_and_CC' and choice == 0:

                            workload_comparison_offloading_time[0] = y

                        elif strategy == 'VCC_and_CC' and choice == 2 and speed == '/speed0':
                            workload_comparison_offloading_time[1] = y
                    elif sim_type == 'users_number':
                        if strategy == 'EC_and_CC' and choice == 0:
                            users_comparison_offloading_time[0] = y

                        elif strategy == 'VCC_and_CC' and choice == 2 and speed == '/speed0':

                            users_comparison_offloading_time[1] = y

                            piechart_for_users_number_offloading_time_division_data = np.append(
                                piechart_for_users_number_offloading_time_division_data, uplink_times[3])
                            piechart_for_users_number_offloading_time_division_data = np.append(
                                piechart_for_users_number_offloading_time_division_data, elaboration_times[3])
                            piechart_for_users_number_offloading_time_division_data = np.append(
                                piechart_for_users_number_offloading_time_division_data, downlink_times[3])

                            # creating piechart for 10 users for contributions of offloading time
                            plt.figure()
                            plt.rcParams.update({'font.size': 18})
                            y_pie_chart = piechart_for_users_number_offloading_time_division_data

                            mylabels_pie_c = ["Uplink", "Elaboration", "Downlink"]
                            colors_pie = [60, 100, 150]
                            plt.pie(y_pie_chart, labels=mylabels_pie_c, autopct='%0.1f%%', pctdistance=0.5,
                                    labeldistance=1.1, colors=[str(x / 255) for x in colors_pie])
                            # plt.legend(title="Contributions of times in offloading", loc='upper right')
                            plt.tight_layout()
                            plt.savefig(output_curve_name + '_pie_chart_' + prd + '.png')
                            # plt.close()

                    ax1.set_yscale('log')
                    # now i put the three class or latency low, mid, high
                    ax1.axhline(y=0.016, color='r')
                    ax1.axhline(y=0.1, color='y')
                    ax1.axhline(y=0.5, color='g')

                    ax1.plot(x_ax, y, linestyle=styles[choice], marker=markers[choice], color=colors[choice],
                             label=paradigms_all[choice])
                    ax1.set_ylabel(y_label, fontsize=fontsize, fontfamily='serif')
                    ax1.set_xlabel(x_label, fontsize=fontsize, fontfamily='serif')
                    ax1.legend(fontsize=ticks_fontsize)

                    if sim_type == 'cars_number' and choice == 2:
                        cars_vcc_x = cars_number_in_vcc_for_any_config(cars_number_files,
                                                                       x_axis, index)
                        ax2 = ax1.twiny()
                        ax2.set_xlim(ax1.get_xlim())
                        ax2.set_xlabel("Cars in VCC", fontsize=fontsize, fontfamily='serif')
                        x_ax2 = [str(x) for x in cars_vcc_x]
                        ax2.set_xticks(x_pos, x_ax2, fontsize=18, rotation=90)

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
                    ax_insto.set_xticks(x_pos, x_ax, fontsize=ticks_fontsize, rotation=rotate_var)
                    plt.yticks(fontsize=ticks_fontsize)
                    ax_insto.set_ylabel(y_label)
                    ax_insto.set_xlabel(x_label)
                    ax_insto.legend()
                    # new_max = max(y)
                    # if new_max > max_y:
                    #    max_y = new_max
                    # fig_insto.savefig(output_curve_name + '_time_instogram_' + prd + '.png')

                # fig.savefig(output_curve_name + ".png", bbox_inches='tight')
                plt.clf()
                plt.cla()
                plt.close()

                satisfied_requests, cloud_perc_requests = plot_paradigm_for_any_config(input_file_names, x_axis, index,
                                                                                       x_label,
                                                                                       output_curve_name)
                if simulation["type"] == "cars_number" and speed == '/speed0':
                    print(cloud_perc_requests)

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
