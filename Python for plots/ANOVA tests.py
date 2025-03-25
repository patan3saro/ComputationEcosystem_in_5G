import pandas as pd
import statsmodels.api as sm
from statsmodels.formula.api import ols
import seaborn as sns
import matplotlib.pyplot as plt

offl_time_workload_EC = [0.00908666, 0.00933804, 0.00959292, 0.00981947, 0.00984704, 0.01010482,
                         0.01037634, 0.01194721, 0.01344004, 0.01505966, 0.01901891, 0.02400481, 0.03067894]
offl_time_users_EC = [0.00955987, 0.00954269, 0.0095252, 0.01015881, 0.01035865, 0.00991965,
                      0.01034862, 0.01012093, 0.00998132, 0.00995325, 0.00995341, 0.00991444, 0.0100511]
offl_time_workload_VCC = [0.020932636132544112, 0.022254189335567137, 0.025217995471845292, 0.02656501421252561,
                          0.02793199136875897, 0.030796112439316955, 0.03357088666317939, 0.04773919843424518,
                          0.06170094114847739, 0.07567180511545929, 0.10374572177511987, 0.13167108560535742,
                          0.15993972467431578]
offl_time_users_VCC = [0.030473195776991552, 0.02662355666273827, 0.02728108951577177, 0.026464849284973665,
                       0.02654086695197214, 0.026447311097848147, 0.026456438524097896, 0.026110714119823207,
                       0.025844007534902784, 0.02579638436935544, 0.02596692279724464, 0.02586865944575179,
                       0.025999424928097214]


offl_time_users_EC_100 = [0.015988642714234284, 0.01557078901336509, 0.015595656776888829, 0.017035170780410717,
                          0.023013139233585275, 0.08494170698154319, 0.0866725949734731, 0.08780004598875678,
                          0.0868448983336564,
                          0.09128427054476078, 4.76531477142636, 8.24821279903287, 7.8342902797929845]


offl_time_users_VCC_100 = [0.04392751484645597, 0.031162124275297912, 0.03265408632638319, 0.04037572979114107,
                           0.04831329852803274, 0.05585565121478598, 0.0679311829206527, 0.0721088541909271,
                           0.08682968608944362, 7.326535837975281, 11.279051903608272, 9.694207131514855,
                           14.2257608764588]

offl_time_[0.02611601 0.02736869 0.02752138 0.0273626 ]
[0.02611601 0.02736869 0.02752138 0.0273626 ]_VCC

users = [1, 2, 5, 10, 15, 20, 25, 30, 40, 50, 60, 80, 100]
workload = [100, 200, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 6000, 8000, 10000]
workload = workload[7:]
# Creiamo un dataset simulato
data = pd.DataFrame({
    'Users': workload * 2,
    'Group': ['VCC'] * len(workload) + ['EC'] * len(workload),  # Due gruppi
    'Performance': offl_time_workload_VCC [7:]+  # VCC
                   offl_time_workload_EC [7:] # EC
})

# ANOVA a una via per l'effetto di Users sulla Performance
model = ols('Performance ~ C(Users)', data=data).fit()
anova_table = sm.stats.anova_lm(model, typ=2)

# Stampiamo i risultati dell'ANOVA
print(anova_table)

# --------- Visualizzazione con Seaborn ---------

# 1. Boxplot per confrontare la distribuzione della performance nei gruppi VCC ed EC
plt.figure(figsize=(10, 6))
sns.boxplot(x='Users', y='Performance', hue='Group', data=data)
plt.title('Distribuzione della Performance per Users e Gruppi')
plt.xlabel('Workload')
plt.ylabel('Performance')
plt.legend(title="Group")
plt.show()

# 2. Grafico a punti per mostrare l'andamento delle performance nei due gruppi
plt.figure(figsize=(10, 6))
sns.pointplot(x='Users', y='Performance', hue='Group', data=data, dodge=True, markers=['o', 's'])
plt.title('Effetto di Users sulla Performance nei due Gruppi')
plt.xlabel('Workload')
plt.ylabel('Performance')
plt.legend(title="Group")
plt.show()
