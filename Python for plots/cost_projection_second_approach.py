import matplotlib.pyplot as plt
import numpy as np
import math
import matplotlib.ticker as ticker

years_list = [1, 3, 5]
cpu_edge_lifespan = 3
one_cpu_cost_edge = 700  # $

requests_cost = 2 * (10 ** (-5))
rate = 5  # req/second
users = 100
time_hours_day_in_seconds = 15 * 3600

# Simplified cost computation
def cost_VCC_function(years, rate, users, time_hours_day_in_seconds, requests_cost):
    return requests_cost * rate * users * time_hours_day_in_seconds * 365 * years

def cost_edge_function(years, cpu_edge_lifespan, one_cpu_cost_edge, rate, users,
                        time_hours_day_in_seconds, requests_cost, beta):
    cpus_number = math.ceil(years / cpu_edge_lifespan)
    capex_ec = one_cpu_cost_edge * cpus_number
    maint_opex = years * (68423 / 50)
    requests_opex = (requests_cost + beta) * rate * users * time_hours_day_in_seconds * 365 * years
    return capex_ec, maint_opex, requests_opex

def plot_stacked_bars(ax, title, beta_values, data):
    bar_width = 0.4 * 0.4  # Reduce the bar width by 40%
    index = np.arange(len(beta_values))

    part1 = np.array([data[b]['part1'] for b in beta_values])
    part2 = np.array([data[b]['part2'] for b in beta_values])
    part3 = np.array([data[b]['part3'] for b in beta_values])

    bars1 = ax.bar(index - bar_width / 2, part1[:, 0], bar_width, label='$Capex_{EC}$', color='0.1')
    bars2 = ax.bar(index - bar_width / 2, part2[:, 0], bar_width, bottom=part1[:, 0], color='0.8', label='$C_{Maint}$')
    bars3 = ax.bar(index - bar_width / 2, part3[:, 0], bar_width, bottom=part1[:, 0] + part2[:, 0], color='0.5', label='$C_{req, tot, EC}$')
    bars4 = ax.bar(index + bar_width / 2, part1[:, 1], bar_width, color='0.3', label='$C_{req, tot, EC}$')

    ax.set_xticks(index)
    ax.set_xticklabels([f'$\\beta$={b:.0e}' for b in beta_values], fontsize=12)
    ax.set_ylabel('Cost [$]', fontsize=16)  # Label font size = 16
    ax.set_title(title, fontsize=16)  # Title font size = 16
    ax.legend(loc='center right', bbox_to_anchor=(1, 0.5), fontsize=12, facecolor='white', frameon=True)


    # Set y-axis to scientific notation with fontsize 12
    ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    for label in ax.get_yticklabels():
        label.set_fontsize(12)

    # Set x-axis limits to reduce its length
    ax.set_xlim(-0.5, len(beta_values) - 0.5)  # Adjust these values to reduce the x-axis length

    # Add EC and VCC labels just above the bars
    for i, beta in enumerate(beta_values):
        # Calculate the height of the bars to position the text
        height = part1[i, 0] + part2[i, 0] + part3[i, 0]  # EC part
        ax.text(index[i] - bar_width / 2, height * 1.02, f'EC', ha='right', fontsize=10.5)

        height_vcc = part1[i, 1]  # VCC part
        ax.text(index[i] + bar_width / 2, height_vcc * 1.02, f'VCC', ha='left', fontsize=10.5)

betas = np.array([-0.000001, 0, 0.000001])

tmp_edge = [[] for _ in range(len(years_list))]
tmp_vcc = [[] for _ in range(len(years_list))]
for year_idx in range(len(years_list)):
    for beta in betas:
        tmp_edge[year_idx].append(cost_edge_function(years_list[year_idx], cpu_edge_lifespan, one_cpu_cost_edge, rate, users,
                                                     time_hours_day_in_seconds, requests_cost, beta))
        tmp_vcc[year_idx].append(cost_VCC_function(years_list[year_idx], rate, users,
                                                   time_hours_day_in_seconds, requests_cost))

# Define data dynamically using betas as keys
data_1y = {beta: {'part1': [tmp_edge[0][i][0], tmp_vcc[0][i]], 'part2': [tmp_edge[0][i][1], 0], 'part3': [tmp_edge[0][i][2], 0]} for i, beta in enumerate(betas)}
data_3y = {beta: {'part1': [tmp_edge[1][i][0], tmp_vcc[1][i]], 'part2': [tmp_edge[1][i][1], 0], 'part3': [tmp_edge[1][i][2], 0]} for i, beta in enumerate(betas)}
data_5y = {beta: {'part1': [tmp_edge[2][i][0], tmp_vcc[2][i]], 'part2': [tmp_edge[2][i][1], 0], 'part3': [tmp_edge[2][i][2], 0]} for i, beta in enumerate(betas)}

# Create a single figure with three subplots
fig, axes = plt.subplots(1, 3, figsize=(12, 6))  # Adjust the size to fit three plots
plot_stacked_bars(axes[0], '1 Year', betas, data_1y)
plot_stacked_bars(axes[1], '3 Years', betas, data_3y)
plot_stacked_bars(axes[2], '5 Years', betas, data_5y)

# Adjust the spacing between subplots
fig.subplots_adjust(wspace=0.3)  # Reduce space between subplots

# Adjust layout to make it look nice
fig.tight_layout()

# Save the combined figure with high resolution
fig.savefig('/home/saro/Desktop/cost_analysis_combined.png', dpi=300)  # High resolution
plt.show()
