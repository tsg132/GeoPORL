import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

# Load all simulation files
sim_files = sorted(Path('.').glob('simulation_*.csv'))

if not sim_files:
    print("No simulation files found!")
    exit(1)

print(f"Found {len(sim_files)} simulation files")

# Read all simulations and store in a list
all_sims = []
for f in sim_files:
    df = pd.read_csv(f)
    all_sims.append(df)

# Find the minimum length across all simulations (in case some terminated early)
min_length = min(len(df) for df in all_sims)

# Truncate all dataframes to the same length
all_sims = [df.iloc[:min_length] for df in all_sims]

# Columns to average and plot
columns_to_plot = ['gdp_pc', 'cohesion', 'internal_conflict_risk', 
                   'shock_severity', 'unemployment', 'inflation']

# Compute mean across all simulations
mean_data = {}
for col in columns_to_plot:
    # Check if column exists in first dataframe
    if col not in all_sims[0].columns:
        print(f"Warning: Column '{col}' not found in data, skipping...")
        continue
    values = np.array([df[col].values for df in all_sims])
    mean_data[col] = values.mean(axis=0)

# Create DataFrame from mean data
mean_df = pd.DataFrame(mean_data)
mean_df['t'] = all_sims[0]['t'].values[:min_length]

# Plot
fig, ax = plt.subplots(figsize=(14, 8))

for col in columns_to_plot:
    ax.plot(mean_df['t'], mean_df[col], label=col, linewidth=2)

ax.set_xlabel('Time', fontsize=12)
ax.set_ylabel('Value', fontsize=12)
ax.set_title(f'Average Dynamics Across {len(sim_files)} Simulations', fontsize=14)
ax.legend(loc='best', fontsize=10)
ax.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('average_simulation.png', dpi=150)
print(f"\nPlot saved to average_simulation.png")

plt.show()
