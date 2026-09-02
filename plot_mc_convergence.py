import csv
import matplotlib.pyplot as plt

with open("mc_convergence.csv") as f:
    first_line = f.readline().strip()
    parts = dict(item.split("=") for item in first_line.split(","))
    bs_price = float(parts["bs_price"])
    bs_delta = float(parts["bs_delta"])

    reader = csv.DictReader(f)
    num_paths, price, delta = [], [], []
    for row in reader:
        num_paths.append(int(row["num_paths"]))
        price.append(float(row["price"]))
        delta.append(float(row["delta"]))

fig, axes = plt.subplots(1, 2, figsize=(12, 5))

axes[0].plot(num_paths, price, marker="o", color="#2563eb", label="Monte Carlo estimate")
axes[0].axhline(y=bs_price, color="#dc2626", linestyle="--", label="Black-Scholes (exact)")
axes[0].set_xscale("log")
axes[0].set_xlabel("Number of simulated paths")
axes[0].set_ylabel("Option price")
axes[0].set_title("Price: Monte Carlo converging to the true value")
axes[0].legend()
axes[0].grid(alpha=0.3)

axes[1].plot(num_paths, delta, marker="o", color="#2563eb", label="Monte Carlo + AD estimate")
axes[1].axhline(y=bs_delta, color="#dc2626", linestyle="--", label="Black-Scholes (exact)")
axes[1].set_xscale("log")
axes[1].set_xlabel("Number of simulated paths")
axes[1].set_ylabel("Delta")
axes[1].set_title("Delta: computed via AD through the simulation")
axes[1].legend()
axes[1].grid(alpha=0.3)

plt.tight_layout()
plt.savefig("mc_convergence_plot.png", dpi=150)
print("Saved mc_convergence_plot.png")