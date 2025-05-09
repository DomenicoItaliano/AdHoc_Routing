import matplotlib.pyplot as plt
import re
import os
import numpy as np

def estrai_dati(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    throughput = 0
    pacchetti_rx = 0
    pacchetti_persi = 0
    pacchetti_tx = 0

    match_throughput = re.search(r'Total Throughput \(Mbps\): ([\d\.]+)', content)
    match_rx = re.search(r'Total Rx Packets: (\d+)', content)
    match_lost = re.search(r'Total Lost Packets: (\d+)', content)
    match_tx = re.search(r'Total Tx Packets: (\d+)', content)

    if match_throughput:
        throughput = float(match_throughput.group(1))
    if match_rx:
        pacchetti_rx = int(match_rx.group(1))
    if match_lost:
        pacchetti_persi = int(match_lost.group(1))
    if match_tx:
        pacchetti_tx = int(match_tx.group(1))

    return throughput, pacchetti_rx, pacchetti_persi, pacchetti_tx

def calcola_media(lista):
    return np.mean(lista)

flows = list(range(1, 10))
throughput_dsdv = []
throughput_aodv = []
rx_dsdv = []
rx_aodv = []
lost_dsdv = []
lost_aodv = []
tx_dsdv = []
tx_aodv = []

for f in flows:
    t_dsdv, r_dsdv, l_dsdv, tx_dsdv_single = [], [], [], []
    t_aodv, r_aodv, l_aodv, tx_aodv_single = [], [], [], []

    for run in range(1, 6):
        dati_dsdv = estrai_dati(f"results/dsdv_flows_{f}_run{run}.txt")
        dati_aodv = estrai_dati(f"results/aodv_flows_{f}_run{run}.txt")

        t_dsdv.append(dati_dsdv[0])
        r_dsdv.append(dati_dsdv[1])
        l_dsdv.append(dati_dsdv[2])
        tx_dsdv_single.append(dati_dsdv[3])

        t_aodv.append(dati_aodv[0])
        r_aodv.append(dati_aodv[1])
        l_aodv.append(dati_aodv[2])
        tx_aodv_single.append(dati_aodv[3])

    throughput_dsdv.append(calcola_media(t_dsdv))
    throughput_aodv.append(calcola_media(t_aodv))
    rx_dsdv.append(calcola_media(r_dsdv))
    rx_aodv.append(calcola_media(r_aodv))
    lost_dsdv.append(calcola_media(l_dsdv))
    lost_aodv.append(calcola_media(l_aodv))
    tx_dsdv.append(calcola_media(tx_dsdv_single))
    tx_aodv.append(calcola_media(tx_aodv_single))

# Normalizzazione dei pacchetti trasmessi
norm_tx_dsdv = [tx/flows[i] for i, tx in enumerate(tx_dsdv)]
norm_tx_aodv = [tx/flows[i] for i, tx in enumerate(tx_aodv)]

# Overhead = (Tx - Rx) / numero flussi
overhead_dsdv = [(tx_dsdv[i] - rx_dsdv[i]) / flows[i] for i in range(len(flows))]
overhead_aodv = [(tx_aodv[i] - rx_aodv[i]) / flows[i] for i in range(len(flows))]

# Plot dei grafici

def plot_graph(y_dsdv, y_aodv, ylabel, title, filename):
    plt.figure()
    plt.plot(flows, y_dsdv, marker='o', label='DSDV')
    plt.plot(flows, y_aodv, marker='s', label='AODV')
    plt.xlabel("Numero di flussi")
    plt.ylabel(ylabel)
    plt.title(title)
    plt.grid(True)
    plt.legend()
    plt.savefig(filename)
    plt.show()

plot_graph(throughput_dsdv, throughput_aodv, "Throughput medio (Mbps)", "Throughput medio vs Numero di flussi", "throughput_vs_flussi_media.png")
plot_graph(rx_dsdv, rx_aodv, "Pacchetti ricevuti", "Pacchetti ricevuti vs Numero di flussi", "pacchetti_ricevuti_vs_flussi_media.png")
plot_graph(lost_dsdv, lost_aodv, "Pacchetti persi", "Pacchetti persi vs Numero di flussi", "pacchetti_persi_vs_flussi_media.png")
plot_graph(norm_tx_dsdv, norm_tx_aodv, "Pacchetti trasmessi normalizzati", "Pacchetti trasmessi normalizzati vs Numero di flussi", "pacchetti_trasmessi_normalizzati_vs_flussi_media.png")
plot_graph(overhead_dsdv, overhead_aodv, "Overhead", "Overhead vs Numero di flussi", "overhead_vs_flussi_media.png")
