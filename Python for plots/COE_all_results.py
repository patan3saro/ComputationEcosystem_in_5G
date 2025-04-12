"""
COE Results Generator

This script automatically generates and processes results from simulation data.
It reads CSV files from specified directories, processes the data, and creates visualizations.

Usage:
    python COE_all_results.py
"""
import csv
import decimal
import matplotlib.pyplot as plt
from pathlib import Path
import numpy as np
import os
import logging
from datetime import datetime
from utils import (
    failure_rate_for_any_config,
    convert_nanoseconds_to_decimal_seconds,
    cars_number_in_vcc_for_any_config,
    plot_paradigm_for_any_config,
    workload_strategy_comparison,
    users_strategy_comparison,
    dwell_time_per_speed_per_vehicle_num,
    plot_uplink_distr
)

# ==============================================
# CONFIGURATION SECTION - Modify as needed
# ==============================================
# Set your working directory here
# For your usage, replace with your own directory:
WORK_DIRECTORY = '/your/directory/FirstConf'

# Logging configuration
log_file = f"coe_results_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(log_file),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

# ==============================================
# SIMULATION PARAMETERS
# ==============================================
RUN_INSTANTS = ['06-13-2024-22:59']
STRATEGIES = ['EC_and_CC', 'VCC_and_CC']
PARADIGMS_ALL = ['EDGE', 'CLOUD', 'VCC']
SUMO_SCENARIOS = ['RectangleScenario']  # Can also include: 'GridScenario'
VEHICLES_NUM = 40
SIMULATION_DURATION = 120  # seconds

# X-axis values for different simulation parameters
X_AXS = [
    (100, 200, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 6000, 8000, 10000),  # Workload [MI]
    (1, 2, 5, 10, 15, 20, 25, 30, 40, 50, 60, 80, 100, 150)  # Users
]

def ensure_output_dirs(path):
    """
    Ensure that the output directory structure exists.

    Args:
        path: Base path for output directories
    """
    Path(path).mkdir(parents=True, exist_ok=True)
    logger.info(f"Created output directory: {path}")

def process_results():
    """Main function to process all simulation results"""
    workload_comparison_offloading_time = [0] * 2
    users_comparison_offloading_time = [0] * 2
    dwell_time_files = []

    logger.info(f"Starting results processing from {WORK_DIRECTORY}")
    logger.info(f"Processing scenarios: {SUMO_SCENARIOS}")

    for strategy_idx, strategy in enumerate(STRATEGIES):
        logger.info(f"Processing strategy: {strategy}")

        # Set up simulation type configurations
        users_number = {
            "type": "users_number",
            "x_axis": X_AXS[1],
            "index": 7,
            "index_failure": 7,
            "x_label": "Users"
        }

        workload = {
            "type": "workload",
            "x_axis": X_AXS[0],
            "index": 13,
            "index_failure": 10,
            "x_label": "Workload [MI]"
        }

        # Strategy-specific configurations
        if strategy == STRATEGIES[0]:  # EC_and_CC
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

        elif strategy == STRATEGIES[1]:  # VCC_and_CC
            paradigms = ('CLOUD', 'VCC')

            cars_number = {
                "type": "cars_number",
                "x_axis": (10, 20, 40, 60),
                "index": 6,
                "index_failure": 6,
                "x_label": "Vehicles"
            }

            car_computation_capacity = 71120
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

        # Process each scenario
        for sumo_scenario in SUMO_SCENARIOS:
            logger.info(f"Processing scenario: {sumo_scenario}")

            for simulation in simulation_types:
                sim_type = simulation["type"]
                logger.info(f"Processing simulation type: {sim_type}")

                x_label = simulation["x_label"]
                x_axis = simulation["x_axis"]
                index = simulation["index"]
                index_failure = simulation["index_failure"]

                for speed in speeds:
                    logger.info(f"Processing speed: {speed}")

                    # Initialize file lists
                    input_file_names = []
                    cars_number_files = []
                    input_total_VCC_sent_packets_files = []
                    input_total_EDGE_sent_packets_files = []
                    input_total_CLOUD_sent_packets_files = []
                    input_from_edge_to_VCC_packets_files = []

                    # Prepare output path
                    output_path = f"{WORK_DIRECTORY}/results/{sumo_scenario}/{strategy}{speed}"
                    ensure_output_dirs(output_path)
                    output_curve_name = f"{output_path}/{simulation['type']}"

                    # Set up figure for plotting
                    fig, ax1 = plt.subplots()
                    all_offloading_per_all_seeds = [[] for _ in range(len(x_axis))]

                    # Process each run instant
                    for run_instant in RUN_INSTANTS:
                        runs = [0, 1, 2, 3, 4, 6, 7, 8, 9] if run_instant == RUN_INSTANTS[0] else []

                        for r in runs:
                            for value in simulation["x_axis"]:
                                # Construct file paths
                                base_path = f"{WORK_DIRECTORY}/csv_files/{sumo_scenario}/{run_instant}/{strategy}{speed}"
                                csv_file = f"{base_path}/offloading_time_{simulation['type']}_{value}_{r}.csv"

                                # Define related file paths
                                csv_file_cars = csv_file.replace('/offloading_time_', '/total_VCC_sent_packets_')
                                input_total_VCC_sent_packets_file = csv_file_cars
                                input_total_EDGE_sent_packets_file = csv_file.replace('/offloading_time_', '/total_EDGE_sent_packets_')
                                input_total_CLOUD_sent_packets_file = csv_file.replace('/offloading_time_', '/total_CLOUD_sent_packets_')
                                input_from_edge_to_VCC_packets_file = csv_file.replace('/offloading_time_', '/total_VCC_received_from_edge_packets_')

                                # Handle special case for cars_number
                                if simulation["type"] == "cars_number" and r == 1:
                                    dwell_time_file = csv_file.replace('/offloading_time_', '/dwell_time_')
                                    dwell_time_files.append(dwell_time_file)
                                elif simulation["type"] == "cars_number" and speed == '/speed0':
                                    file_used_cars_number[int(simulation["x_axis"].index(value))][r] = csv_file

                                # Collect file paths
                                input_file_names.append(csv_file)
                                cars_number_files.append(csv_file_cars)
                                input_total_VCC_sent_packets_files.append(input_total_VCC_sent_packets_file)
                                input_total_EDGE_sent_packets_files.append(input_total_EDGE_sent_packets_file)
                                input_total_CLOUD_sent_packets_files.append(input_total_CLOUD_sent_packets_file)
                                input_from_edge_to_VCC_packets_files.append(input_from_edge_to_VCC_packets_file)

                    # Process each paradigm
                    for prd in paradigms:
                        choice = PARADIGMS_ALL.index(prd)
                        len_x = len(simulation["x_axis"])

                        # Initialize arrays for data collection
                        z = np.zeros(len_x)
                        y = np.zeros(len_x)  # Offloading time
                        uplink_times = np.zeros(len_x)
                        queueing_times = np.zeros(len_x)
                        elaboration_times = np.zeros(len_x)
                        downlink_times = np.zeros(len_x)
                        a = [[] for _ in range(len_x)]

                        # Process each input file
                        for input_file_name in input_file_names:
                            try:
                                with open(input_file_name) as file:
                                    csvreader = csv.reader(file)

                                    for row in csvreader:
                                        paradigm = int(row[4]) == choice

                                        # Calculate times
                                        offloading_time = float(float(row[3]) - convert_nanoseconds_to_decimal_seconds(row[2]))
                                        uplink_time = float(convert_nanoseconds_to_decimal_seconds(row[8]) - convert_nanoseconds_to_decimal_seconds(row[2]))
                                        elaboration_time = float(convert_nanoseconds_to_decimal_seconds(row[9]))
                                        queueing_time = float(convert_nanoseconds_to_decimal_seconds(row[12])) - float(convert_nanoseconds_to_decimal_seconds(row[8]))
                                        downlink_time = float(float(row[3]) - convert_nanoseconds_to_decimal_seconds(row[12])) - elaboration_time

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
                                                    logger.warning(f"Negative downlink time detected: {downlink_times[k]}")
                                                    logger.warning(f"Details: choice={choice}, offloading_time={offloading_time}, queueing_time={queueing_time}, uplink_time={uplink_time}, elaboration_time={elaboration_time}")
                                                    logger.warning(f"Row: {row}")
                            except Exception as e:
                                logger.error(f"Error processing file {input_file_name}: {e}")

                        # Avoid division by zero
                        z[z == 0] = 1

                        # Calculate averages
                        y /= z
                        uplink_times /= z
                        queueing_times /= z
                        elaboration_times /= z
                        downlink_times /= z

                        # Plotting parameters
                        styles = (':', '-', '--')
                        markers = ('o', '+', 's')
                        colors = ('0.2', '0.5', '0.8')
                        fontsize = 12
                        label_font_size = 16

                        # Prepare x-axis labels
                        x_ax = [str(int(x)) for x in simulation["x_axis"]]
                        x_pos = [x for x in range(len_x)]
                        y_label = 'Offloading time [s]'

                        # Set plot formatting
                        if sim_type in ('workload', 'vcc_resources', 'edge_resources', 'users_number'):
                            plt.xticks(rotation=45)
                        else:
                            plt.xticks(fontsize=fontsize, rotation=0)
                            plt.yticks(fontsize=fontsize)

                        plt.gcf().subplots_adjust(left=0.2, bottom=0.25)

                        # Save workload and users comparison data for later
                        if sim_type == 'workload':
                            if strategy == 'EC_and_CC' and choice == 0:
                                workload_comparison_offloading_time[0] = y
                        elif sim_type == 'users_number':
                            plt.xticks(rotation=45)
                            if strategy == 'EC_and_CC' and choice == 0:
                                users_comparison_offloading_time[0] = y

                            # Create percentage chart for VCC
                            if strategy == 'VCC_and_CC' and choice == 2:
                                width = 0.6
                                fig_insto_perc, ax_insto_perc = plt.subplots()

                                total_times = uplink_times + elaboration_times + downlink_times
                                uplink_times_perc = 100 * (uplink_times / total_times)
                                elaboration_times_perc = 100 * (elaboration_times / total_times)
                                dowlink_times_perc = 100 * (downlink_times / total_times)

                                # Create stacked bar chart
                                ax_insto_perc.bar(x_ax, uplink_times_perc, width, label='Uplink')
                                ax_insto_perc.bar(x_ax, elaboration_times_perc, width, bottom=uplink_times_perc, label='Elaboration')
                                ax_insto_perc.bar(x_ax, dowlink_times_perc, width, label='Downlink', bottom=uplink_times_perc + elaboration_times_perc)

                                # Set formatting
                                rotate_var = 45
                                ax_insto_perc.set_xticks(x_pos, x_ax, fontsize=fontsize, rotation=rotate_var)
                                plt.yticks(fontsize=fontsize)
                                ax_insto_perc.set_ylabel("Offloading Time [%]", fontsize=label_font_size)
                                ax_insto_perc.set_xlabel('Users', fontsize=label_font_size)
                                ax_insto_perc.legend(fontsize=fontsize)
                                plt.tight_layout()
                                # Uncomment to save:
                                # fig_insto_perc.savefig(f"{output_curve_name}_percentage_insto_{prd}.png")
                                plt.close(fig_insto_perc)

                        # Configure main plot
                        ax1.set_yscale('log')
                        ax1.axhline(y=0.016, color='r', linestyle=':')
                        ax1.axhline(y=0.1, color='y', linestyle='--')
                        ax1.axhline(y=0.5, color='g')

                        # Plot data
                        if sim_type == 'cars_number' and choice == 2:
                            ax1.plot(x_ax, y, linestyle=styles[choice], marker=markers[choice], color=colors[choice], label='VCCFirst')
                            logger.info(f"Cars number: {y}")
                            ax1.set_ylim(max(y) + 0.1)
                        elif not (sim_type == 'cars_number' and choice == 1):
                            ax1.plot(x_ax, y, linestyle=styles[choice], marker=markers[choice], color=colors[choice], label=PARADIGMS_ALL[choice])

                        # Set labels and legend
                        ax1.set_ylabel(y_label, fontsize=label_font_size, fontfamily='serif')
                        ax1.set_xlabel(x_label, fontsize=label_font_size, fontfamily='serif')
                        ax1.legend(fontsize=fontsize)

                        # Add secondary axis for cars_number
                        if sim_type == 'cars_number' and choice == 2:
                            cars_vcc_x = cars_number_in_vcc_for_any_config(cars_number_files, x_axis, index)
                            ax2 = ax1.twiny()
                            ax2.set_xlim(ax1.get_xlim())
                            ax2.set_xlabel("Average used vehicles", fontsize=label_font_size, fontfamily='serif')
                            x_ax2 = [str(x) for x in cars_vcc_x]
                            ax2.set_xticks(x_pos, x_ax2, fontsize=fontsize, rotation=90)

                        # Create time division histogram
                        width = 0.6
                        fig_insto, ax_insto = plt.subplots()

                        # Create stacked bars
                        ax_insto.bar(x_ax, uplink_times, width, label='Uplink')
                        ax_insto.bar(x_ax, queueing_times, width, label='Queueing', bottom=uplink_times)
                        ax_insto.bar(x_ax, elaboration_times, width, bottom=uplink_times + queueing_times, label='Elaboration')
                        ax_insto.bar(x_ax, downlink_times, width, label='Downlink', bottom=uplink_times + queueing_times + elaboration_times)

                        # Format histogram
                        rotate_var = 0 if sim_type != 'workload' else 45
                        ax_insto.set_xticks(x_pos, x_ax, fontsize=fontsize, rotation=rotate_var)
                        plt.yticks(fontsize=fontsize)
                        ax_insto.set_ylabel(y_label, fontsize=label_font_size)
                        ax_insto.set_xlabel(x_label, fontsize=label_font_size)
                        ax_insto.legend(fontsize=fontsize)
                        plt.tight_layout()

                        # Save histogram (uncomment to enable)
                        # fig_insto.savefig(f"{output_curve_name}_time_instogram_{prd}.png", bbox_inches='tight')
                        plt.close(fig_insto)

                    # Process percentiles for VCC_and_CC
                    if strategy == 'VCC_and_CC':
                        avgs = []
                        percentiles_90 = []
                        percentiles_95 = []
                        percentiles_99 = []

                        for lista in all_offloading_per_all_seeds:
                            if lista:  # Check if list is not empty
                                sorted_offloading_values = np.sort(lista)
                                percentile_90 = np.percentile(sorted_offloading_values, 90)
                                percentile_95 = np.percentile(sorted_offloading_values, 95)
                                percentile_99 = np.percentile(sorted_offloading_values, 99)
                                average = sum(lista) / len(lista)
                            else:
                                percentile_90 = percentile_95 = percentile_99 = average = 0

                            percentiles_90.append(percentile_90)
                            percentiles_95.append(percentile_95)
                            percentiles_99.append(percentile_99)
                            avgs.append(average)

                        # Save for comparison
                        if sim_type == 'workload' and speed == '/speed0':
                            workload_comparison_offloading_time[1] = avgs
                        elif sim_type == "users_number" and speed == '/speed0':
                            users_comparison_offloading_time[1] = avgs

                        # Create percentile plot
                        fig, ax_total = plt.subplots()
                        ax_total.set_ylabel(y_label, fontsize=label_font_size, fontfamily='serif')
                        ax_total.set_xlabel(x_label, fontsize=label_font_size, fontfamily='serif')
                        ax_total.set_xticks(x_pos, x_ax, fontsize=fontsize)
                        ax_total.set_yscale('log')

                        # Add horizontal lines for latency classes
                        ax_total.axhline(y=0.016, color='r', linestyle=':')
                        ax_total.axhline(y=0.1, color='y', linestyle='--')

                        # Add special annotations for VCC resources
                        if sim_type == "vcc_resources":
                            vertical_lines = [
                                {"Raspberry Pi 3": 2441},
                                {"Raspberry Pi 4": 7568},
                                {"Raspberry Pi 5": 13231},
                                {"ARM Cortex A73": 71120}
                            ]

                            font_props = {'family': 'serif', 'color': 'black', 'size': 12}

                            for line in vertical_lines:
                                for key, value in line.items():
                                    plt.axvline(x=value, color='k', linestyle='--')
                                    ymin, ymax = plt.gca().get_ylim()
                                    y_position = ymin + 0.2 * (ymax - ymin)
                                    plt.text(value - 0.5, y_position, key, fontdict=font_props, rotation=90, va='center')

                        # Add secondary axis for cars_number
                        if sim_type == 'cars_number' and choice == 2 and speed == '/speed0':
                            cars_vcc_x = cars_number_in_vcc_for_any_config(cars_number_files, x_axis, index)
                            ax_total2 = ax_total.twiny()
                            ax_total2.set_xlim(ax_total.get_xlim())
                            ax_total2.set_xlabel("Average used vehicles", fontsize=label_font_size, fontfamily='serif')
                            x_ax2 = [str(x) for x in cars_vcc_x]
                            ax_total2.set_xticks(x_pos, x_ax2, fontsize=fontsize, rotation=90)

                        # Plot percentiles
                        ax_total.plot(x_ax, avgs, linestyle=styles[choice], marker='o', color='0.6', label='Mean')
                        ax_total.plot(x_ax, percentiles_90, linestyle='--', marker='.', color='0.2', label='90 percentile')
                        ax_total.plot(x_ax, percentiles_95, linestyle=':', marker='x', color='0.5', label='95 percentile')
                        ax_total.plot(x_ax, percentiles_99, linestyle='-.', marker='*', color='0.8', label='99 percentile')

                        # Finalize plot
                        ax_total.set_ylabel(y_label, fontsize=label_font_size, fontfamily='serif')
                        ax_total.set_xlabel(x_label, fontsize=label_font_size, fontfamily='serif')
                        ax_total.legend(fontsize=fontsize)

                        # Save plot (uncomment to enable)
                        # fig.savefig(f"{output_curve_name}_total.png", bbox_inches='tight')
                        plt.close(fig)

                        logger.info(f"Processed {sim_type} data: {y}")

                    # Process paradigm distribution
                    satisfied_requests, cloud_perc_requests = plot_paradigm_for_any_config(
                        input_file_names, x_axis, index, x_label, output_curve_name
                    )

                    # Process failure rates (uncomment to enable)
                    # failure_rate_for_any_config(
                    #     input_total_VCC_sent_packets_files,
                    #     input_total_EDGE_sent_packets_files,
                    #     input_total_CLOUD_sent_packets_files,
                    #     input_from_edge_to_VCC_packets_files,
                    #     index_failure, x_axis,
                    #     x_label, output_curve_name,
                    #     satisfied_requests
                    # )

                    # Close main figure
                    plt.close(fig)

    # Process dwell time data
    logger.info("Processing dwell time data")
    dwell_time_per_speed_per_vehicle_num(
        dwell_time_files, VEHICLES_NUM, speeds, WORK_DIRECTORY,
        SIMULATION_DURATION, SUMO_SCENARIOS[0]
    )

    # Process strategy comparisons
    logger.info("Processing strategy comparisons")
    workload_strategy_comparison(
        workload_comparison_offloading_time, X_AXS[0], "Workload [MI]",
        STRATEGIES, WORK_DIRECTORY, SUMO_SCENARIOS[0]
    )

    users_strategy_comparison(
        users_comparison_offloading_time, X_AXS[1], "Users",
        STRATEGIES, WORK_DIRECTORY, SUMO_SCENARIOS[0]
    )

    logger.info("Results processing complete")

if __name__ == "__main__":
    process_results()
