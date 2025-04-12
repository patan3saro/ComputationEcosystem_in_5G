import math
import pandas as pd
from matplotlib import pyplot as plt
import utils

# Define working directory where simulation results are stored
work_directory = '/home/saro/Desktop/FirstConfReqRate-06-08-2024'

# Define offloading strategies to be compared
strategies = ['EC_and_CC', 'VCC_and_CC']  # Edge Computing and Cloud Computing vs Vehicular Cloud Computing and Cloud Computing
paradigms_all = ['EDGE', 'CLOUD', 'VCC']  # All possible computation paradigms

# Define column names for CSV parsing
column_names = [
    'id_user', 'size', 'time_initial', 'time_final', 'paradigm',
    'interarrival_time', 'cars_number', 'users_number', 'uplink_instant', 
    'elaboration_time', 'edge_resources', 'vcc_resources', 'queuing_instant', 
    'workload', 'used_cars', 'vehicle_queue_length'
]

# Pre-calculated offloading time results for 5 requests per second scenario
# These are average offloading times in seconds for different user counts
EC_5_per_s = [0.00955987, 0.00954269, 0.0095252, 0.01015881, 0.01035865, 0.00991965,
              0.01034862, 0.01012093, 0.00998132, 0.00995325, 0.00995341, 0.00991444,
              0.0100511]  # Edge Computing at 5 req/s

VCC_rate_5_per_s = [0.030473195776991552, 0.02662355666273827, 0.02728108951577177, 0.026464849284973665,
                    0.02654086695197214, 0.026447311097848147, 0.026456438524097896, 0.026110714119823207, 
                    0.025844007534902784, 0.02579638436935544, 0.02596692279724464, 0.02586865944575179, 
                    0.025999424928097214]  # Vehicular Cloud Computing at 5 req/s

# Combine pre-calculated results for easy access
offload_list_5_per_sec = [EC_5_per_s, VCC_rate_5_per_s]

# Plot formatting parameters
fontsize = 12
label_font_size = 16

# Simulation parameters
seeds = [0]  # Simulation seed(s) to analyze
users = [1, 2, 5, 10, 15, 20, 25, 30, 40, 50, 60, 80, 100]  # User counts to analyze
x_ax = [str(int(x)) for x in users]  # X-axis labels
x_pos = [x for x in range(len(users))]  # X-axis positions
rates = [10]  # Request rates per second to analyze (100 req/s = 10 req/s per user * 10 users)

# Plot styling parameters
colors = ('0.05', '0.3', '0.55', '0.8')  # Gray scales for different lines
styles = (':', '-', '--', '-.')  # Line styles
markers = ('o', '+', 's', '*')  # Point markers

# Loop through each strategy (ECFirst and VCCFirst)
for s in range(len(strategies)):
    for rate in rates:
        y_values = [0] * len(users)
        cloud_usage_array = [0] * len(users)
        
        # Loop through each simulation seed
        for seed in seeds:
            # Loop through each user count
            for i in range(len(users)):
                # Construct file paths for the current configuration
                offloading_file = f"{work_directory}/csv_files/{strategies[s]}/speed0/offloading_time_rate_users_{rate}_{users[i]}_{seed}.csv"
                total_sent_file = f"{work_directory}/csv_files/{strategies[s]}/speed0/total_sent_packets_rate_users_{rate}_{users[i]}_{seed}.csv"
                total_sent_edge = f"{work_directory}/csv_files/{strategies[s]}/speed0/total_EDGE_sent_packets_rate_users_{rate}_{users[i]}_{seed}.csv"
                total_sent_cloud = f"{work_directory}/csv_files/{strategies[s]}/speed0/total_CLOUD_sent_packets_rate_users_{rate}_{users[i]}_{seed}.csv"

                # Read offloading time data from CSV
                df = pd.read_csv(offloading_file, skiprows=20, names=column_names)

                # Count occurrences of each paradigm (EDGE=0, CLOUD=1, VCC=2)
                count_values = df['paradigm'].value_counts()

                # Get the count for primary paradigm (EDGE for ECFirst, VCC for VCCFirst)
                if s == 0:  # ECFirst strategy
                    count_FIRST = count_values.get(0, 0)  # Count of EDGE usage
                elif s == 1:  # VCCFirst strategy
                    count_FIRST = count_values.get(2, 0)  # Count of VCC usage
                
                # Get count of CLOUD usage
                count_CLOUD = count_values.get(1, 0)

                # Calculate percentage of Cloud usage
                if count_CLOUD == 0:
                    perc_usage_CLOUD = 0
                else:
                    perc_usage_CLOUD = count_CLOUD / df['paradigm'].shape[0]
                
                cloud_usage_array[i] = round(100 * perc_usage_CLOUD)

                # Convert nanosecond timestamps to seconds
                df['time_initial'] = df['time_initial'].apply(utils.convert_nanoseconds_to_decimal_seconds)
                
                # Calculate offloading time (time_final is already in seconds)
                df['offloading_time'] = df['time_final'] - df['time_initial']

                # Calculate average offloading time
                average_offloading = df['offloading_time'].mean()
                y_values[i] = average_offloading

    # Set legend label based on strategy
    if s == 0:
        legend = 'ECFirst'
    else:
        legend = 'VCCFirst'

    # Plot results for 100 req/s (current analysis)
    plt.plot(x_ax, y_values, 
             label=f"{legend} 100 req/s", 
             marker=markers[s], 
             color=colors[s],
             linestyle=styles[s])
    
    print(y_values, legend)
    
    # Plot pre-calculated results for 5 req/s
    plt.plot(x_ax, offload_list_5_per_sec[s], 
             label=f"{legend} 5 req/s", 
             marker=markers[s + 2], 
             color=colors[s + 2],
             linestyle=styles[s + 2])
    
    # Add horizontal lines for latency requirements
    plt.axhline(y=0.016, color='r', linestyle=':', label='LL++')  # Extremely Low Latency (16ms)
    plt.axhline(y=0.1, color='y', linestyle='--', label='LL+')   # Very Low Latency (100ms)
    plt.axhline(y=0.5, color='g', label='LL')                    # Low Latency (500ms)
    
    # Format plot axes
    plt.xticks(x_pos, x_ax, fontsize=fontsize)
    plt.yticks(fontsize=fontsize)
    plt.yscale('log')  # Logarithmic scale for better visualization
    
    # Add axis labels
    plt.xlabel('Users', fontsize=label_font_size)
    plt.ylabel('Offloading time [s]', fontsize=label_font_size)
    
    # Add legend
    plt.legend(fontsize=fontsize)
    
    # Print cloud usage percentage by user count
    print(strategies[s], cloud_usage_array)
    
    plt.tight_layout()
    plt.draw()

# Uncomment to save the figure
# plt.savefig('/home/saro/Desktop/requests_rate_comparison_users.png', bbox_inches='tight')

# Display the plot
plt.show()

# Previous run results for reference:
# EC_and_CC [0, 0, 0, 0, 0, 38, 50, 63, 70, 72, 76, 83, 86]
# VCC_and_CC [0, 0, 0, 16, 29, 42, 62, 69, 78, 84, 87, 93, 99]
