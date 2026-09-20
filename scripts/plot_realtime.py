import sys, csv
import numpy as np
import matplotlib.pyplot as plt

rows = list(csv.DictReader(open(sys.argv[1]) if len(sys.argv) > 1 else sys.stdin))
def col(n): return np.array([float(r[n]) for r in rows])

t = col('t')
tx, ty = col('true_x'), col('true_y')
ex, ey = col('est_x'), col('est_y')
sx = col('sig_x')
delivered = col('delivered')
link_bad = col('link_bad')

fig, ax = plt.subplots(2, 1, figsize=(14, 8), sharex=True)

ax[0].plot(t, tx, 'g-', label='справжня x')
ax[0].plot(t, ex, 'b-', lw=1.2, label='оцінка x')
ax[0].fill_between(t, ex - 3*sx, ex + 3*sx, color='b', alpha=0.15, label='±3σ')
ax[0].scatter(t[delivered == 1], ex[delivered == 1], s=10, c='k', label='прийнято кадр')
ax[0].legend(); ax[0].set_title('x(t): самотактований приймач на рваному радіо')

ax[1].fill_between(t, 0, 1, where=(link_bad == 1), color='r', alpha=0.3, step='mid')
ax[1].set_yticks([]); ax[1].set_title('стан каналу (червоне = bad, кластери втрат)')
ax[1].set_xlabel('t, с')

plt.tight_layout()
out = (sys.argv[1].rsplit('.', 1)[0] + '.png') if len(sys.argv) > 1 else 'realtime.png'
plt.savefig(out, dpi=120); print('saved', out)