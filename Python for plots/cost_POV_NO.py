import math

import numpy as np
from matplotlib import pyplot as plt

years_list = range(1, 21)
cpu_edge_lifespan = 3
cpu_vehicles_lifespan = 12
one_cpu_cost_edge = 700  # $

requests_cost = 2 * (10 ** (-5))
requests_cost_VCC=[]
ratio = []
n = 0.5
while n <= 1.1:
    ratio.append(round(n, 1))  # Arrotonda alla prima cifra decimale
    requests_cost_VCC.append(n * requests_cost)
    n += 0.1
rate = 5  # req/second
users = 100
time_hours_day_in_seconds = 15 * 3600


def cost_edge_fuction(years, cpu_edge_lifespan, one_cpu_cost_edge, rate, users,
                      time_hours_day_in_seconds, requests_cost, number_of_edge_nodes_per_technician):
    cpus_number = math.ceil(years / cpu_edge_lifespan)
    euipement_cost_edge = one_cpu_cost_edge * cpus_number

    total_cost_edge = requests_cost * rate * users * time_hours_day_in_seconds * 365 * years + years * 68423 / number_of_edge_nodes_per_technician + euipement_cost_edge

    return total_cost_edge


def cost_VCC_fuction(years, rate, users,
                     time_hours_day_in_seconds, requests_cost_VCC):
    total_cost_VCC = requests_cost_VCC * rate * users * time_hours_day_in_seconds * 365 * years

    return total_cost_VCC


################################
tmp_edge = []
number_of_edge_nodes_per_technician = 10
for years in years_list:
    tmp_edge.append(cost_edge_fuction(years, cpu_edge_lifespan, one_cpu_cost_edge, rate, users,
                                      time_hours_day_in_seconds, requests_cost, number_of_edge_nodes_per_technician))

plt.figure(figsize=(6, 3))
plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
for idx in range(len(requests_cost_VCC)):
    cost_VCC = requests_cost_VCC[idx]
    rat = ratio[idx]
    print(ratio[idx])
    tmp_vcc = []
    for years in years_list:
        tmp_vcc.append(cost_VCC_fuction(years, rate, users,
                                        time_hours_day_in_seconds, cost_VCC))

    plt.plot(np.array(years_list), np.array(tmp_edge) - np.array(tmp_vcc), label=str(rat) + " * $c_{EC-req}$ ",
             marker='+')
    plt.legend()
    plt.draw()

plt.axhline(0, color='0.1', linestyle='--')
# Set x and y labels
plt.ylabel('$c_{EC}-c_{VCC}$ [\$]', fontsize=15)
plt.xlabel('Investment time [years]', fontsize=12)
custom_xtick_labels = [str(x) for x in years_list]
# Set custom xtick labels
plt.xticks(years_list, custom_xtick_labels, fontsize=12)
plt.yticks(fontsize=12)
plt.tight_layout()

plt.savefig("/home/saro/Desktop/cost_NO.png")
plt.show()
