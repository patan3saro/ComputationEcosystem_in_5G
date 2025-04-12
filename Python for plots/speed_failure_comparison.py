import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Define working directory containing the CSV files to be analyzed
work_directory = '/yourpath/FirstConf-28-06-2024-speed'

# Define vehicle speed categories that will be analyzed
speeds = ('speed0', 'speed1', 'speed2')  # Corresponding to 13.1, 50, and 100 km/h
runs = [0, 1, 3, 4, 5, 7, 8, 9, 10, 11]  # 10 simulation seeds for statistical validity

# Initialize arrays to store failure metrics across different stages of the offloading process
# Each array will hold data for the three different speeds
failure_rate_ped_total_sent_req = np.zeros(3)     # Total requests sent by pedestrians
failure_BS_received_requests = np.zeros(3)        # Requests received by the Base Station
failure_VCC_received_requests = np.zeros(3)       # Requests received by the Vehicular Cloud Computing system
failure_VCC_sent_to_BS_requests = np.zeros(3)     # Responses sent back from VCC to the BS
failure_VCC_rejected = np.zeros(3)                # Requests rejected by VCC due to full queue
offloaded = np.zeros(3)                           # Successfully offloaded and completed requests
failure_BS_ped = np.zeros(3)                      # Failures between BS and pedestrians

# Loop through each speed category
for j in range(len(speeds)):
    # Loop through each simulation run (seed)
    for r in runs:
        # Construct the base filename for this speed and run
        csv_file = f'{work_directory}/csv_files/{speeds[j]}/offloading_time_{speeds[j]}_{r}.csv'

        # Construct filenames for different data files by replacing parts of the base filename
        input_total_VCC_sent_packets_file = csv_file.replace('/offloading_time_', '/total_VCC_sent_packets_')
        input_total_sent_packets_file = csv_file.replace('/offloading_time_', '/total_sent_packets_')
        input_from_edge_to_VCC_packets_file = csv_file.replace('/offloading_time_',
                                                               '/total_VCC_received_from_edge_packets_')
        input_rejected_VCC_packets_file = csv_file.replace('/offloading_time_',
                                                           '/total_VCC_rejected_packets_cause_full_queue_')
        input_BD_to_ped_packets_file = csv_file.replace('/offloading_time_',
                                                        '/response_offloading_passing_in_gNB_')

        # Read each CSV file into a pandas DataFrame
        df_ped_total_requests = pd.read_csv(input_total_sent_packets_file)
        df_BS_received_requests = pd.read_csv(input_total_VCC_sent_packets_file)
        df_VCC_received_requests = pd.read_csv(input_from_edge_to_VCC_packets_file)
        
        # Filter the responses to only include those targeting the specific pedestrian ID (4028)
        df_VCC_sent_to_BS_requests = pd.read_csv(input_BD_to_ped_packets_file)
        df_VCC_sent_to_BS_requests = df_VCC_sent_to_BS_requests[df_VCC_sent_to_BS_requests.iloc[:, 1] == 4028]
        
        df_rejected = pd.read_csv(input_rejected_VCC_packets_file)
        df_offloaded = pd.read_csv(csv_file)

        # Count rows in each DataFrame and add to the respective array
        # Each row represents a packet/request at different stages
        failure_rate_ped_total_sent_req[j] += df_ped_total_requests.shape[0]
        failure_BS_received_requests[j] += df_BS_received_requests.shape[0]
        failure_VCC_received_requests[j] += df_VCC_received_requests.shape[0]
        failure_VCC_sent_to_BS_requests[j] += df_VCC_sent_to_BS_requests.shape[0]
        failure_VCC_rejected[j] += df_rejected.shape[0]
        offloaded[j] += df_offloaded.shape[0]

# Calculate average values across all runs
len_runs = len(runs)
failure_rate_ped_total_sent_req = failure_rate_ped_total_sent_req / len_runs
failure_BS_received_requests = failure_BS_received_requests / len_runs
failure_VCC_received_requests = failure_VCC_received_requests / len_runs
failure_VCC_sent_to_BS_requests = failure_VCC_sent_to_BS_requests / len_runs
failure_VCC_rejected = failure_VCC_rejected / len_runs
offloaded = offloaded / len_runs

# Calculate failure percentages at different stages of the offloading process
# 1. Failure from user to gNB (Base Station)
failure_1_2 = 100 * (failure_rate_ped_total_sent_req - failure_BS_received_requests) / failure_rate_ped_total_sent_req

# 2. Failure from gNB to VCC 
failure_2_3 = 100 * (failure_BS_received_requests - failure_VCC_received_requests) / failure_rate_ped_total_sent_req

# 3. VCC rejected requests due to full queue
failure_rejected = 100 * failure_VCC_rejected / failure_rate_ped_total_sent_req

# 4. Failure in VCC response back to gNB
failure_3_4 = 100 * (failure_VCC_received_requests - failure_VCC_sent_to_BS_requests) / failure_rate_ped_total_sent_req

# 5. Failure from gNB to user (final response)
failure_4_5 = 100 * (failure_VCC_sent_to_BS_requests - offloaded) / failure_rate_ped_total_sent_req

# Create the stacked bar chart
ind = np.arange(len(speeds))  # X positions for the bars
width = 0.35                  # Width of the bars

fig, ax = plt.subplots(figsize=(10, 6))
plt.ylim(0, 5)  # Set y-axis limit to 5% for better visualization

# Create stacked bars for each failure type
bar1 = ax.bar(ind, failure_1_2, width, label='User to gNB (requests)')
bar2 = ax.bar(ind, failure_2_3, width, bottom=failure_1_2, label='gNB to VCC (requests)')
bar3 = ax.bar(ind, failure_rejected, width, bottom=failure_1_2 + failure_2_3, label='gNB to VCC (rejected requests)')
bar4 = ax.bar(ind, failure_3_4, width, bottom=failure_1_2 + failure_2_3 + failure_rejected,
              label='VCC to gNB (response)')
bar5 = ax.bar(ind, failure_4_5, width, bottom=failure_1_2 + failure_2_3 + failure_rejected + failure_3_4,
              label='gNB to User (response)')

# Set font sizes for better readability
fontsize = 12
label_font_size = 16

# Add labels, title and legend
ax.set_ylabel('Failure rate [%]', fontsize=label_font_size)
ax.set_xlabel('Average speed of vehicles [km/h]', fontsize=label_font_size)
# Map speed identifiers to actual km/h values
velocities = ('13.1', '50', '100')
ax.set_xticks(ind)
plt.yticks(fontsize=14)
ax.set_xticklabels(velocities, fontsize=fontsize)
ax.legend(loc='upper left', framealpha=0.2)

# Save the figure
plt.savefig('/home/saro/Desktop/FirstConf-28-06-2024-speed/speeds.png', dpi=300, bbox_inches='tight')

# Display the plot
plt.show()
