import matplotlib.pyplot as plt

# Dati per l'istogramma
users = ['150']
ec_first = [0.01]
vcc_first = [0.03]

# Larghezza delle barre
bar_width = 0.35

# Posizioni delle barre
r1 = [0]  # Posizione della prima barra
r2 = [x + bar_width for x in r1]  # Posizione della seconda barra

# Creazione dell'istogramma
plt.bar(r1, ec_first, color='blue', width=bar_width, edgecolor='grey', label='ECFirst')
plt.bar(r2, vcc_first, color='green', width=bar_width, edgecolor='grey', label='VCCFirst')
plt.yscale('log')

# Aggiunta delle linee orizzontali
plt.axhline(y=0.016, color='r', linestyle='--')
plt.axhline(y=0.1, color='y', linestyle='-.')
plt.axhline(y=0.5, color='g', linestyle='-')

# Etichettatura degli assi con dimensione e tipo di carattere
plt.xlabel('Users', fontsize=14, fontname='serif')
plt.ylabel('Offloading time[s]', fontsize=14, fontname='serif')

# Aggiunta delle etichette per l'asse x con dimensione e tipo di carattere
plt.xticks([r + bar_width / 2 for r in range(len(users))], users, fontsize=12, fontname='serif')

# Aggiunta della legenda
plt.legend()

# Salvataggio della figura (cambiare il percorso in base al sistema)
plt.savefig('/home/saro/Desktop/istogramma.png')

# Visualizzazione del grafico
plt.show()
