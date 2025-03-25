import csv
import decimal
import statistics
import numpy as np
from matplotlib import pyplot as plt
from math import sqrt


def convert_nanoseconds_to_decimal_seconds(x):
    res = -40
    try:
        value2 = x[1:].replace('ns', '')
        res = float(value2) * float(0.000000001)
    except:
        print('Error in conversion: the converted item is:', x)
    return res


def cars_number_in_vcc_for_any_config(input_files_VCC, x_axs, idx):
    m = np.zeros(len(x_axs))
    cars_in_vcc_x = np.zeros(len(x_axs))
    for input_file_VCC in input_files_VCC:
        f = open(input_file_VCC)
        csvredr = csv.reader(f)
        for rw in csvredr:

            for k in range(len(x_axs)):
                cars_in_vcc_i = x_axs[k] - int(rw[11])
                if x_axs[k] == int(float(rw[idx])):
                    m[k] += 1
                    cars_in_vcc_x[k] += cars_in_vcc_i

    tmp = cars_in_vcc_x / m
    cars_in_vcc_x = [format(x, '.2f') for x in tmp]
    return cars_in_vcc_x


def plot_paradigm_for_any_config(input_file_names, x_axis, index, x_label, output_curve_name):
    edge_satisfied_requests = np.zeros(len(x_axis))
    cloud_satisfied_requests = np.zeros(len(x_axis))
    vcc_satisfied_requests = np.zeros(len(x_axis))
    amount_of_files = len(input_file_names)
    avg = np.ones(len(x_axis)) * amount_of_files

    for in_file in input_file_names:
        file = open(in_file)

        csvreader = csv.reader(file)
        for row in csvreader:

            for k in range(len(x_axis)):
                if x_axis[k] == int(float(row[index])):
                    paradigm = int(row[4])
                    if paradigm == 0:  # Edge
                        edge_satisfied_requests[k] += 1
                    elif paradigm == 1:  # cloud
                        cloud_satisfied_requests[k] += 1
                    elif paradigm == 2:  # VCC
                        vcc_satisfied_requests[k] += 1

    width = 0.6
    x_ax = [str(x) for x in x_axis]
    x_pos = [x for x in range(len(x_axis))]
    y_label = 'Total satisfied requests [%]'

    fig, ax = plt.subplots()
    vcc_satisfied_requests /= avg
    cloud_satisfied_requests /= avg
    edge_satisfied_requests /= avg
    total_satisfied_requests = vcc_satisfied_requests + cloud_satisfied_requests + edge_satisfied_requests
    vcc_perc_requests = vcc_satisfied_requests / total_satisfied_requests
    cloud_perc_requests = cloud_satisfied_requests / total_satisfied_requests
    edge_perc_requests = edge_satisfied_requests / total_satisfied_requests
    vcc_perc_requests *= 100
    cloud_perc_requests *= 100
    edge_perc_requests *= 100
    ax.bar(x_ax, vcc_perc_requests, width, label='Vehicular Cloud')

    ax.bar(x_ax, edge_perc_requests, width, label='Edge', bottom=np.array(vcc_perc_requests))
    ax.bar(x_ax, cloud_perc_requests, width,
           bottom=np.array(edge_perc_requests) + np.array(vcc_perc_requests),
           label='Cloud')
    plt.xticks(x_pos, x_ax)
    ax.set_ylabel(y_label)
    ax.set_xlabel(x_label)
    ax.legend()
    print(x_label, cloud_perc_requests)
    # fig.savefig(output_curve_name + '_instogram_satisfied_for_any_config' + '.png')
    plt.clf()
    plt.cla()
    plt.close()
    file.close()
    numbers_of_satified_requests = vcc_satisfied_requests, edge_satisfied_requests, cloud_satisfied_requests
    return numbers_of_satified_requests, cloud_perc_requests


def failure_rate_for_any_config(input_total_VCC_sent_packets_files, input_total_EDGE_sent_packets_files,
                                input_total_CLOUD_sent_packets_files, input_from_edge_to_VCC_packets_files,
                                index_failure, x_axis,
                                x_label, output_curve_name,
                                satisfied_requests):
    total_requests_from_edge_to_vcc_per_config = np.zeros(len(x_axis))

    len_files_VCC_0 = len(input_from_edge_to_VCC_packets_files)
    if len_files_VCC_0 == 0:
        VCC_failed_requests_without_mobility = np.zeros(len_files_VCC_0)
    else:
        for input_from_edge_to_VCC_packets_file in input_from_edge_to_VCC_packets_files:
            # now we calculate the part of failure due to the mobility
            try:
                file_total_requests_from_edge_to_vcc = open(input_from_edge_to_VCC_packets_file)
                csvreader_total_requests_from_edge_to_vcc = csv.reader(file_total_requests_from_edge_to_vcc)

                for row in csvreader_total_requests_from_edge_to_vcc:

                    for k in range(len(x_axis)):
                        if x_axis[k] == int(float(row[index_failure])):
                            total_requests_from_edge_to_vcc_per_config[k] += 1
            except:
                total_requests_from_edge_to_vcc_per_config += np.zeros(len(x_axis))

            total_requests_from_edge_to_vcc_per_config /= len_files_VCC_0
            delta_0 = total_requests_from_edge_to_vcc_per_config - satisfied_requests[0]
            if delta_0.any() >= 0 and total_requests_from_edge_to_vcc_per_config.all() != 0:
                mobility_failure_requests = delta_0 * 100 / np.array(total_requests_from_edge_to_vcc_per_config)
            else:
                mobility_failure_requests = np.zeros(len_files_VCC_0)

        total_packets_in_VCC_per_config = np.zeros(len(x_axis))
        len_files_VCC = len(input_total_VCC_sent_packets_files)

    if len_files_VCC == 0:
        VCC_failed_requests_without_mobility = np.zeros(len_files_VCC)
    else:
        for input_total_VCC_sent_packets_file in input_total_VCC_sent_packets_files:
            try:
                file_VCC = open(input_total_VCC_sent_packets_file)
                csvreader_VCC = csv.reader(file_VCC)

                for row in csvreader_VCC:

                    for k in range(len(x_axis)):
                        if x_axis[k] == int(float(row[index_failure])):
                            total_packets_in_VCC_per_config[k] += 1
            except:
                total_packets_in_VCC_per_config += np.zeros(len(x_axis))

        total_packets_in_VCC_per_config /= len_files_VCC
        delta_1 = total_packets_in_VCC_per_config - satisfied_requests[0]
        if delta_1.any() >= 0 and total_packets_in_VCC_per_config.all() != 0:
            VCC_failed_requests_without_mobility = delta_1 * 100 / total_packets_in_VCC_per_config
        else:
            print("ERROR in failure config delta 1")
            VCC_failed_requests_without_mobility = np.zeros(len(x_axis))
        VCC_failed_requests_without_mobility -= mobility_failure_requests

    total_packets_in_EDGE_per_config = np.zeros(len(x_axis))
    len_files_EDGE = len(input_total_EDGE_sent_packets_files)

    if len_files_EDGE == 0:
        EDGE_failed_requests = np.zeros(len_files_EDGE)
    else:
        for input_total_EDGE_sent_packets_file in input_total_EDGE_sent_packets_files:
            try:
                file_EDGE = open(input_total_EDGE_sent_packets_file)
                csvreader_EDGE = csv.reader(file_EDGE)

                for row in csvreader_EDGE:

                    for k in range(len(x_axis)):
                        if x_axis[k] == int(float(row[index_failure])):
                            total_packets_in_EDGE_per_config[k] += 1
            except:
                total_packets_in_EDGE_per_config += np.zeros(len(x_axis))

        total_packets_in_EDGE_per_config /= len_files_EDGE
        delta_2 = total_packets_in_EDGE_per_config - satisfied_requests[1]
        if delta_2.any() >= 0 and total_packets_in_EDGE_per_config.all() != 0:
            EDGE_failed_requests = delta_2 * 100 / total_packets_in_EDGE_per_config
        else:
            print("ERROR in failure config delta 2")
            EDGE_failed_requests = np.zeros(len_files_EDGE)

    total_packets_in_CLOUD_per_config = np.zeros(len(x_axis))
    len_files_CLOUD = len(input_total_CLOUD_sent_packets_files)

    if len_files_CLOUD == 0:
        CLOUD_failed_requests = np.zeros(len_files_CLOUD)
    else:
        for input_total_CLOUD_sent_packets_file in input_total_CLOUD_sent_packets_files:
            try:
                file_CLOUD = open(input_total_CLOUD_sent_packets_file)
                csvreader_CLOUD = csv.reader(file_CLOUD)

                for row in csvreader_CLOUD:

                    for k in range(len(x_axis)):
                        if x_axis[k] == int(float(row[index_failure])):
                            total_packets_in_CLOUD_per_config[k] += 1
            except:
                total_packets_in_CLOUD_per_config += np.zeros(len(x_axis))

        total_packets_in_CLOUD_per_config /= len_files_CLOUD
        delta_3 = total_packets_in_CLOUD_per_config - satisfied_requests[2]
        if delta_3.any() >= 0 and total_packets_in_CLOUD_per_config.all() != 0:
            CLOUD_failed_requests = delta_3 * 100 / total_packets_in_CLOUD_per_config
        else:
            print("ERROR in failure config delta 1")
            CLOUD_failed_requests = np.zeros(len_files_CLOUD)

    width = 0.6
    x_ax = [str(x) for x in x_axis]
    x_pos = [x for x in range(len(x_axis))]
    y_label = 'Failed requests [%]'

    fig, ax = plt.subplots()
    fontsize = 12
    label_font_size = 16
    ax.set_ylabel(y_label, fontsize=label_font_size)
    ax.set_xlabel(x_label, fontsize=label_font_size)
    plt.xticks(fontsize=fontsize)
    plt.yticks(fontsize=fontsize)
    plt.xticks(rotation=30)
    plt.tight_layout()

    ax.bar(x_ax, VCC_failed_requests_without_mobility, width, label='Vehicular Cloud not caused by mobility')
    ax.bar(x_ax, mobility_failure_requests, width, label='Vehicular Cloud caused by mobility',
           bottom=VCC_failed_requests_without_mobility)
    ax.bar(x_ax, EDGE_failed_requests, width, label='Edge',
           bottom=VCC_failed_requests_without_mobility + mobility_failure_requests)
    ax.bar(x_ax, CLOUD_failed_requests, width,
           bottom=VCC_failed_requests_without_mobility + mobility_failure_requests +
                  EDGE_failed_requests,
           label='Cloud')
    plt.xticks(x_pos, x_ax)
    plt.ylim(ymin=0, ymax=100)
    ax.set_ylabel(y_label)
    ax.set_xlabel(x_label)
    ax.legend(fontsize=fontsize - 2)
    plt.tight_layout()
    # fig.savefig(output_curve_name + '_instogram_paradigm_failure_for_any_config' + '.png')
    plt.clf()
    plt.cla()
    plt.close()


def workload_strategy_comparison(y, x_ax, x_label, strategies, work_directory, sumo_scenario):
    output_path = work_directory + '/results/' + sumo_scenario + '/comparison_workload'
    strategies = ['ECFirst', 'VCCFirst']
    y_label = 'Offloading time [s]'

    styles = (':', '-', '--')  # for plotting different styles of lines styles
    markers = ('o', '+', 's')  # for plotting different styles of lines markers
    colors = ('0.2', '0.5', '0.8')  # for plotting different styles of lines colors
    fontsize = 12
    label_font_size = 16
    plt.clf()
    plt.cla()
    plt.close()
    plt.figure()
    plt.xticks(fontsize=fontsize, rotation=45)
    plt.yticks(fontsize=fontsize)
    plt.gcf().subplots_adjust(left=0.2, bottom=0.25)
    plt.yscale('log')
    # now i put the three class or latency low, mid, high
    plt.axhline(y=0.016, color='r', linestyle=':')
    plt.axhline(y=0.1, color='y', linestyle='--')
    plt.axhline(y=0.5, color='g')
    # plt.text(0.1, 0.016, 'LL++', color='black', va='top', ha='right')
    # plt.text(0.09, 0.1, 'LL+', color='black', va='top', ha='right')
    # plt.text(0.09, 0.5, 'LL', color='black', va='top', ha='right')
    plt.ylabel(y_label, fontsize=label_font_size, fontfamily='serif')
    plt.xlabel(x_label, fontsize=label_font_size, fontfamily='serif')
    x_ax = [str(x) for x in x_ax]
    for choice in range(len(strategies)):
        plt.plot(x_ax, y[choice], linestyle=styles[choice], marker=markers[choice], color=colors[choice],
                 label=strategies[choice])
        print("workload comparison " + str(choice) + "  ", y[choice])

        plt.legend(fontsize=fontsize)
        plt.tight_layout()
        plt.draw()
    # plt.savefig(output_path + ".png")

    plt.clf()
    plt.cla()
    plt.close()


def users_strategy_comparison(y, x_ax, x_label, strategies, work_directory, sumo_scenario):
    strategies = ['ECFirst', 'VCCFirst']
    output_path = work_directory + '/results/' + sumo_scenario + '/comparison_users'
    y_label = 'Offloading time [s]'

    styles = (':', '-', '--')  # for plotting different styles of lines styles
    markers = ('o', '+', 's')  # for plotting different styles of lines markers
    colors = ('0.2', '0.5', '0.8')  # for plotting different styles of lines colors
    fontsize = 12
    label_font_size = 16
    plt.clf()
    plt.cla()
    plt.close()
    plt.figure()
    plt.xticks(fontsize=fontsize)
    plt.yticks(fontsize=fontsize)
    plt.gcf().subplots_adjust(left=0.2, bottom=0.25)
    plt.yscale('log')
    # now i put the three class or latency low, mid, high

    plt.axhline(y=0.016, color='r', linestyle=':')
    plt.axhline(y=0.1, color='y', linestyle='--')
    plt.axhline(y=0.5, color='g')
    # plt.text(0.1, 0.016, 'LL++', color='black', va='top', ha='right')
    # plt.text(0.06, 0.1, 'LL+', color='black', va='top', ha='right')
    # plt.text(0.04, 0.5, 'LL', color='black', va='top', ha='right')
    plt.ylabel(y_label, fontsize=label_font_size, fontfamily='serif')
    plt.xlabel(x_label, fontsize=label_font_size, fontfamily='serif')
    x_ax = [str(x) for x in x_ax]
    for choice in range(len(strategies)):
        plt.plot(x_ax, y[choice], linestyle=styles[choice], marker=markers[choice], color=colors[choice],
                 label=strategies[choice])
        print("users comparison " + str(choice) + "  ", y[choice])

        plt.legend(fontsize=fontsize)
        plt.tight_layout()
        plt.draw()
    # plt.savefig(output_path + ".png")
    plt.clf()
    plt.cla()
    plt.close()


def dwell_time_per_speed_per_vehicle_num(input_files, vehicles_num, speeds, work_directory, simulation_duration,
                                         sumo_scenario):
    plt.clf()
    plt.cla()
    plt.close()
    plt.figure()
    output_path = work_directory + '/results/' + sumo_scenario

    start = 5
    step = 10
    stop = 120 + step

    intervals = range(start, stop, step)

    x_ax = [x for x in intervals]

    for speed in speeds:
        avg_dwell_time_per_10_seconds = np.zeros(int(simulation_duration / step) + 1)
        for k in range(len(x_ax)):
            dwell_times = []
            for input_file in input_files:
                if speed in input_file:
                    file = open(input_file)

                    csvreader = csv.reader(file)
                    for row in csvreader:
                        record_time = convert_nanoseconds_to_decimal_seconds(row[3])
                        if vehicles_num == int(float(row[1])) and x_ax[k] - step <= float(record_time) < x_ax[k]:

                            if row[4] == 'IN':

                                entry_time = convert_nanoseconds_to_decimal_seconds(row[3])
                                current_car_address = row[2]

                                file2 = open(input_file)

                                csvreader2 = csv.reader(file2)

                                for elem in csvreader2:
                                    exit_time = convert_nanoseconds_to_decimal_seconds(elem[3])

                                    if current_car_address == elem[2] and elem[4] == 'OUT' and exit_time > entry_time:
                                        dwell_time = (exit_time - entry_time) / simulation_duration
                                        dwell_times.append(dwell_time)
                                        break

            avg_dwell_time_per_10_seconds[k] = np.mean(dwell_times)

        plt.plot(x_ax, avg_dwell_time_per_10_seconds, linestyle=':', marker='+', label=speed)
    plt.legend()
    # plt.savefig(output_path + '/dwell_time_three_speeds.png')


def request_rate_edge_to_vcc(input_file, x_axis, index):
    file = open(input_file)
    csvreader = csv.reader(file)
    requests = np.zeros(len(x_axis))
    request_rate = np.zeros(len(x_axis))
    for row in csvreader:
        for k in range(len(x_axis)):

            if x_axis[k] == int(float(row[index])):
                requests[k] += 1
                print(requests)
    sim_duration = 120
    request_rate /= sim_duration
    print('Request rate', request_rate)
    return request_rate


def plot_confidence_interval(x, values, z=1.96, color='#2187bb', horizontal_line_width=0.25):
    # to avoid problems with standard deviation which wants at least two element
    if len(values) == 1:
        values = values * 2
    # to avoid also problems with means
    elif not values:
        values = [0] * 2
    mean = statistics.mean(values)
    stdev = statistics.stdev(values)

    confidence_interval = z * stdev / sqrt(len(values))

    left = float(x) - horizontal_line_width / 2
    top = mean - confidence_interval
    right = float(x) + horizontal_line_width / 2
    bottom = mean + confidence_interval
    plt.plot([x, x], [top, bottom], color=color)
    # plt.plot([left, right], [top, top], color=color)
    # plt.plot([left, right], [bottom, bottom], color=color)
    plt.plot(x, mean, 'o', color='#f44336')

    return mean, confidence_interval


def failure_rate(version, speed, strategy, input_file_name, input_file_name_MOBILITY_failure, sim_type,
                 output_curve_name):
    # total sent packets
    with open(
            r'../NS3/ns-3-dev/scratch/' + version + '/csv_files/' + speed + '/' + strategy + '/total_sent_packets_' + sim_type + '.csv',
            'r') as fp:
        total_requests = len(fp.readlines())

    file = open(input_file_name)
    file_total_requests_from_edge_to_vcc = open(input_file_name_MOBILITY_failure)
    total_requests_from_edge_to_vcc = len(file_total_requests_from_edge_to_vcc.readlines())

    csvreader = csv.reader(file)

    edge_satisfied = 0
    cloud_satisfied = 0
    VCC_satisfied = 0

    for row in csvreader:
        if int(row[4]) == 0:
            edge_satisfied += 1
        elif int(row[4]) == 1:
            cloud_satisfied += 1
        elif int(row[4]) == 2:
            VCC_satisfied += 1

    unsatisfied_caused_by_mobility = total_requests_from_edge_to_vcc - VCC_satisfied
    unsatisfied_without_mobility = total_requests - (
            edge_satisfied + cloud_satisfied + VCC_satisfied) - unsatisfied_caused_by_mobility

    labels = 'Unsatisfied no mobility', 'Unsatisfied mobility', 'Edge', 'Cloud', 'VCC'
    sizes = [unsatisfied_without_mobility, unsatisfied_caused_by_mobility, edge_satisfied, cloud_satisfied,
             VCC_satisfied]

    fig, ax = plt.subplots()
    #    ax.pie(sizes, labels=labels, autopct='%1.1f%%', colors=['0.2', '0.4', '0.6', '0.8'])
    #    ax.pie(sizes, labels=labels, autopct='%1.1f%%')
    # fig.savefig(output_curve_name + "_pie_chart.png")
    plt.clf()
    plt.cla()
    plt.close()

    file.close()


def plot_uplink_distr(x, uplinks):
    plt.plot(x, uplinks)
    plt.xlabel('Index')
    plt.ylabel('Difference')
    plt.title('Difference Between Consecutive Numbers')
    plt.show()
    exit(0)


"""def graph_vehicles_used_over_time(input_files):
""    x_ax = [x for x in range(120)]
    for row in input_files:
        for file_run in row:
            file = open(file_run)
            csvreader = csv.reader(file)
            for file_row in csvreader:
                
                y_used_vehicles.append(int(file_row[6])-int(file_row[14]))
            y_used_vehicles = 
            x_time = """
