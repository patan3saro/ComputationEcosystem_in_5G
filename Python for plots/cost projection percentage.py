import matplotlib.pyplot as plt
import numpy as np
import math
import matplotlib.ticker as ticker
import os

# Configuration parameters from the paper's cost model (Section 7)
years_list = [1, 3, 5]  # Investment duration in years
cpu_edge_lifespan = 3   # Edge CPU lifespan in years
one_cpu_cost_edge = 700 # Edge CPU cost in $

# Task offloading costs
requests_cost = 2 * (10 ** (-5))  # $ per request
rate = 5  # Requests per second per user
time_hours_day_in_seconds = 15 * 3600  # 15 active hours per day in seconds

# Beta values for sensitivity analysis (VCC bonus)
betas = np.array([0, 0.000001, 0.000002])

def cost_VCC_function(years, rate, users, time_hours_day_in_seconds, requests_cost):
    """Calculate total VCC cost (Equation 6 from paper)"""
    # VCC has only OPEX: cost per request × number of requests
    return requests_cost * rate * users * time_hours_day_in_seconds * 365 * years

def cost_edge_function(years, cpu_edge_lifespan, one_cpu_cost_edge, rate, users,
                      time_hours_day_in_seconds, requests_cost, beta):
    """Calculate EC costs (CAPEX and OPEX) (Equations 4-5 from paper)"""
    # CAPEX: CPU replacement
    cpus_number = math.ceil(years / cpu_edge_lifespan)
    capex_ec = one_cpu_cost_edge * cpus_number

    # OPEX: maintenance
    maint_opex = years * (68423 / 50)  # Annual maintenance cost

    # OPEX: request processing
    requests_opex = (requests_cost + beta) * rate * users * time_hours_day_in_seconds * 365 * years

    return capex_ec, maint_opex, requests_opex

def plot_stacked_bars(ax, title, beta_values, data, users_count):
    """Create stacked bar chart comparing EC and VCC costs (similar to Fig 8 in paper)"""
    bar_width = 0.4 * 0.4
    index = np.arange(len(beta_values))

    part1 = np.array([data[b]['part1'] for b in beta_values])
    part2 = np.array([data[b]['part2'] for b in beta_values])
    part3 = np.array([data[b]['part3'] for b in beta_values])

    # EC bars (CAPEX, maintenance OPEX, request OPEX)
    bars1 = ax.bar(index - bar_width / 2, part1[:, 0], bar_width, label='$c_{CAPEX-EC}$', color='0.1')
    bars2 = ax.bar(index - bar_width / 2, part2[:, 0], bar_width, bottom=part1[:, 0], color='0.8',
                   label='$c_{OPEX-EC, main}$')
    bars3 = ax.bar(index - bar_width / 2, part3[:, 0], bar_width, bottom=part1[:, 0] + part2[:, 0],
                   color='0.5', label='$c_{OPEX-EC, req}$')

    # VCC bars (only OPEX)
    bars4 = ax.bar(index + bar_width / 2, part1[:, 1], bar_width, color='0.3',
                   label='$c_{OPEX-VCC, req}$')

    # Formatting
    ax.set_xticks(index)
    ax.set_xticklabels([f'$\\beta$={b:.0e}' for b in beta_values], fontsize=14)
    ax.set_title(title, fontsize=16)
    # Adjust legend position based on number of users
    if users_count == 1:
        # For 1 user, position legend higher
        ax.legend(loc='upper right', bbox_to_anchor=(1, 0.90), fontsize=14,
                  facecolor='white', frameon=True, framealpha=0.5)
    else:
        # For 100 users, position legend lower right
        ax.legend(loc='lower right', bbox_to_anchor=(1, 0.05), fontsize=14,
                  facecolor='white', frameon=True, framealpha=0.5)

    # Scientific notation for y-axis
    ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    formatter = ticker.ScalarFormatter(useMathText=True)
    formatter.set_scientific(True)
    formatter.set_powerlimits((0, 0))
    ax.yaxis.set_major_formatter(formatter)

    # Exponent font
    offset_text = ax.yaxis.get_offset_text()
    offset_text.set_fontsize(14)
    for label in ax.get_yticklabels():
        label.set_fontsize(16)

    ax.set_xlim(-0.5, len(beta_values) - 0.5)

    # EC and VCC labels above bars
    for i, beta in enumerate(beta_values):
        height = part1[i, 0] + part2[i, 0] + part3[i, 0]  # EC
        ax.text(index[i] - bar_width / 2, height * 1.02, '$c_{EC}$', ha='right', fontsize=14)

        height_vcc = part1[i, 1]  # VCC
        ax.text(index[i] + bar_width / 2, height_vcc * 1.02, '$c_{VCC}$', ha='left', fontsize=14)

def calculate_percentages(cost_data):
    """Calculate percentage distribution of costs for EC and VCC (Tables 6-7 in paper)"""
    # Calculate totals for each beta value
    total_edge_costs = np.array([cost_data[b]['part1'][0] + cost_data[b]['part2'][0] + cost_data[b]['part3'][0]
                                for b in betas])
    total_vcc_costs = np.array([cost_data[b]['part1'][1] for b in betas])

    # Percentages for EC and VCC components
    percentages_edge = {
        'c_{CAPEX-EC}': np.array([cost_data[b]['part1'][0] / total_edge_costs[i] * 100
                                 for i, b in enumerate(betas)]),
        'c_{OPEX-EC, main}': np.array([cost_data[b]['part2'][0] / total_edge_costs[i] * 100
                                      for i, b in enumerate(betas)]),
        'c_{OPEX-EC, req}': np.array([cost_data[b]['part3'][0] / total_edge_costs[i] * 100
                                     for i, b in enumerate(betas)]),
    }

    percentages_vcc = {
        'c_{OPEX-VCC, req}': np.array([cost_data[b]['part1'][1] / total_vcc_costs[i] * 100
                                      for i, b in enumerate(betas)]),
    }

    return percentages_edge, percentages_vcc

def generate_latex_table(percentages_edge, percentages_vcc):
    """Generate LaTeX table for percentage distribution of costs"""
    table_header = r"""
    \begin{table}[h!]
    \centering
    \begin{tabular}{|c|c|c|c|c|}
    \hline
    \textbf{$\beta$} & \textbf{Capex$_{EC}$ (\%)} & \textbf{C$_{Maint}$ (\%)} & \textbf{C$_{req, tot, EC}$ (\%)} & \textbf{Capex$_{VCC}$ (\%)} \\
    \hline
    """

    table_footer = r"""
    \hline
    \end{tabular}
    \caption{Percentage distribution of costs for Edge Computing and VCC.}
    \end{table}
    """

    # Table rows
    rows = []
    for i, beta in enumerate(betas):
        row = f"{beta:.0e} & {percentages_edge['c_{CAPEX-EC}'][i]:.2f} & " \
              f"{percentages_edge['c_{OPEX-EC, main}'][i]:.2f} & " \
              f"{percentages_edge['c_{OPEX-EC, req}'][i]:.2f} & " \
              f"{percentages_vcc['c_{OPEX-VCC, req}'][i]:.2f} \\\\"
        rows.append(row)

    table = table_header + "\n".join(rows) + table_footer
    return table

def run_simulation(users_count):
    """Run cost projection simulation for a specific number of users"""
    print(f"\n=== Running simulation for {users_count} users ===")

    # Initialize data storage
    tmp_edge = [[] for _ in range(len(years_list))]
    tmp_vcc = [[] for _ in range(len(years_list))]

    # Calculate costs for each year and beta value
    for year_idx in range(len(years_list)):
        for beta in betas:
            # Calculate EC costs (CAPEX and OPEX)
            tmp_edge[year_idx].append(
                cost_edge_function(years_list[year_idx], cpu_edge_lifespan, one_cpu_cost_edge,
                                  rate, users_count, time_hours_day_in_seconds, requests_cost, beta)
            )

            # Calculate VCC costs (OPEX only)
            tmp_vcc[year_idx].append(
                cost_VCC_function(years_list[year_idx], rate, users_count,
                                time_hours_day_in_seconds, requests_cost)
            )

    # Organize data for visualization
    data_1y = {beta: {'part1': [tmp_edge[0][i][0], tmp_vcc[0][i]],
                      'part2': [tmp_edge[0][i][1], 0],
                      'part3': [tmp_edge[0][i][2], 0]}
              for i, beta in enumerate(betas)}

    data_3y = {beta: {'part1': [tmp_edge[1][i][0], tmp_vcc[1][i]],
                      'part2': [tmp_edge[1][i][1], 0],
                      'part3': [tmp_edge[1][i][2], 0]}
              for i, beta in enumerate(betas)}

    data_5y = {beta: {'part1': [tmp_edge[2][i][0], tmp_vcc[2][i]],
                      'part2': [tmp_edge[2][i][1], 0],
                      'part3': [tmp_edge[2][i][2], 0]}
              for i, beta in enumerate(betas)}

    # Create figure with three subplots
    fig, axes = plt.subplots(1, 3, figsize=(12, 6))
    plot_stacked_bars(axes[0], '1 Year', betas, data_1y, users_count)
    plot_stacked_bars(axes[1], '3 Years', betas, data_3y, users_count)
    plot_stacked_bars(axes[2], '5 Years', betas, data_5y, users_count)

    # Y-axis label for first subplot
    axes[0].set_ylabel('Cost [$]', fontsize=17)

    # X-axis label for second subplot
    axes[1].set_xlabel('[$/request]', fontsize=16)

    # Title with user count
    fig.suptitle(f'Cost Analysis for {users_count*5} [Requests/second]', fontsize=18)

    # Adjust layout
    plt.tight_layout(rect=[0, 0, 1, 0.96])

    # Create output directory if it doesn't exist
    output_dir = os.path.expanduser('~/Documents/results')
    os.makedirs(output_dir, exist_ok=True)

    # Save the figure
    output_path = os.path.join(output_dir, f'cost_analysis_combined_{users_count}_users.png')
    fig.savefig(output_path, dpi=300)
    print(f"Saved figure to {output_path}")

    # Calculate and display percentage distributions
    percentages_1y_edge, percentages_1y_vcc = calculate_percentages(data_1y)
    percentages_3y_edge, percentages_3y_vcc = calculate_percentages(data_3y)
    percentages_5y_edge, percentages_5y_vcc = calculate_percentages(data_5y)

    # Generate LaTeX table
    latex_table = generate_latex_table(percentages_1y_edge, percentages_1y_vcc)
    print(f"\nLaTeX Table for 1 Year Percentage Distribution ({users_count} users):")
    print(latex_table)

    plt.show()

    return data_1y, data_3y, data_5y

def main():
    """Main function to run simulations for different user counts"""
    # Run for 1 user (as in paper's default scenario)
    data_1_user = run_simulation(1)

    # Run for 100 users (representing high-demand scenario)
    data_100_users = run_simulation(100)

    # Compare the two scenarios
    print("\n=== Comparison between 1 user and 100 users ===")
    print("As shown in the paper, costs scale with number of users, but VCC remains cost-effective")

    # Example comparison for 1 year and beta=0
    beta_zero = betas[0]
    ratio_ec = (data_100_users[0][beta_zero]['part1'][0] +
                data_100_users[0][beta_zero]['part2'][0] +
                data_100_users[0][beta_zero]['part3'][0]) / \
               (data_1_user[0][beta_zero]['part1'][0] +
                data_1_user[0][beta_zero]['part2'][0] +
                data_1_user[0][beta_zero]['part3'][0])

    ratio_vcc = data_100_users[0][beta_zero]['part1'][1] / data_1_user[0][beta_zero]['part1'][1]

    print(f"For 1 year and beta=0:")
    print(f"  - EC cost ratio (100 users/1 user): {ratio_ec:.2f}")
    print(f"  - VCC cost ratio (100 users/1 user): {ratio_vcc:.2f}")

    # Show when VCC becomes more cost-effective than EC
    for year_idx, year in enumerate(years_list):
        for i, beta in enumerate(betas):
            ec_cost_1 = (data_1_user[year_idx][beta]['part1'][0] +
                         data_1_user[year_idx][beta]['part2'][0] +
                         data_1_user[year_idx][beta]['part3'][0])
            vcc_cost_1 = data_1_user[year_idx][beta]['part1'][1]

            ec_cost_100 = (data_100_users[year_idx][beta]['part1'][0] +
                           data_100_users[year_idx][beta]['part2'][0] +
                           data_100_users[year_idx][beta]['part3'][0])
            vcc_cost_100 = data_100_users[year_idx][beta]['part1'][1]

            print(f"\nFor {year} year(s) and beta={beta:.1e}:")
            print(f"  - 1 user: EC=${ec_cost_1:.2f}, VCC=${vcc_cost_1:.2f}, Diff=${ec_cost_1-vcc_cost_1:.2f}")
            print(f"  - 100 users: EC=${ec_cost_100:.2f}, VCC=${vcc_cost_100:.2f}, Diff=${ec_cost_100-vcc_cost_100:.2f}")

if __name__ == "__main__":
    main()
