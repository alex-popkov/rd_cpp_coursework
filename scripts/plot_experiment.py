import sys, csv
import numpy as np
import matplotlib.pyplot as plt

rows = list(csv.DictReader(open(sys.argv[1]) if len(sys.argv) > 1 else sys.stdin))
def col(n): return np.array([float(r[n]) if r[n] != '' else np.nan for r in rows])

t = col('t')
tx, ty = col('true_x'), col('true_y')
mx, my = col('meas_x'), col('meas_y')
ex, ey = col('est_x'), col('est_y')
sx = col('sig_x')

fig, ax = plt.subplots(1, 2, figsize=(14, 6))

ax[0].plot(tx, ty, 'g-', lw=1.5, label='справжня')
ax[0].scatter(mx, my, s=12, c='gray', alpha=0.5, label='прийняті')
ax[0].plot(ex, ey, 'b-', lw=1.2, label='оцінка')
ax[0].set_aspect('equal'); ax[0].legend(); ax[0].set_title('Траєкторія (XY)')

ax[1].plot(t, tx, 'g-', label='справжня x')
ax[1].plot(t, ex, 'b-', label='оцінка x')
ax[1].fill_between(t, ex - 3*sx, ex + 3*sx, color='b', alpha=0.15, label='±3σ')
ax[1].scatter(t, mx, s=10, c='gray', alpha=0.5, label='прийняті x')
ax[1].legend(); ax[1].set_title('x(t) з коридором ±3σ')

plt.tight_layout()
out = (sys.argv[1].rsplit('.', 1)[0] + '.png') if len(sys.argv) > 1 else 'experiment.png'
plt.savefig(out, dpi=120)
print('saved', out)