import random

# Definisci le lettere e le loro probabilità di apparizione
letters = ['A', 'B', 'C', 'D', 'E']
probabilities = [0.1, 0.3, 0.3, 0.2, 0.1]

# Genera la sequenza di 100 elementi
sequence = random.choices(letters, probabilities, k=100)

# Converti la lista in una stringa
sequence_str = ''.join(sequence)

print(sequence)



import random

# Definisci le lettere e le loro probabilità di apparizione
letters = [x for x in range(1, 9)]
probabilities = [x/8 for x in range(1, 9)]

# Genera la sequenza di 100 elementi
sequence = random.choices(letters, probabilities, k=100)


print(sequence)
