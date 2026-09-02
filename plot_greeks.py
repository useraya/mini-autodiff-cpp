import csv
import matplotlib.pyplot as plt

with open("greeks_data.csv") as f:
    first_line = f.readline().strip()
    K = float(first_line.split("=")[1])
    reader = csv.DictReader(f)
    S, price, delta, gamma, vega, rho, theta = [], [], [], [], [], [], []
    for row in reader:
        S.append(float(row["S"]))
        price.append(float(row["price"]))
        delta.append(float(row["delta"]))
        gamma.append(float(row["gamma"]))
        vega.append(float(row["vega"]))
        rho.append(float(row["rho"]))
        theta.append(float(row["theta"]))

fig, axes = plt.subplots(2, 3, figsize=(15, 8))
fig.suptitle(f"Call Price and Greeks vs Stock Price (K={K:.0f}, computed via AD)", fontsize=13)

plots = [
    (axes[0, 0], price, "Price"),
    (axes[0, 1], delta, "Delta"),
    (axes[0, 2], gamma, "Gamma"),
    (axes[1, 0], vega, "Vega"),
    (axes[1, 1], rho, "Rho"),
    (axes[1, 2], theta, "Theta"),
]

for ax, values, title in plots:
    ax.plot(S, values, linewidth=2, color="#2563eb")
    ax.axvline(x=K, color="gray", linestyle="--", linewidth=1, label=f"Strike (K={K:.0f})")
    ax.set_title(title)
    ax.set_xlabel("Stock price (S)")
    ax.grid(alpha=0.3)

axes[0, 0].legend(fontsize=8)

plt.tight_layout()
plt.savefig("greeks_plot.png", dpi=150)
print("Saved greeks_plot.png")