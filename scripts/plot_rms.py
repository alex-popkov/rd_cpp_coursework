import sys, csv
import matplotlib.pyplot as plt

rows = list(csv.DictReader(open(sys.argv[1]) if len(sys.argv) > 1 else sys.stdin))
loss = [float(r['loss']) * 100 for r in rows]
rms  = [float(r['rms']) for r in rows]

plt.figure(figsize=(7, 5))
plt.plot(loss, rms, 'o-')
plt.xlabel('Рівень втрат, %')
plt.ylabel('RMS помилки позиції, м')
plt.title('Деградація оцінки з рівнем втрат')
plt.grid(True, alpha=0.3)
plt.tight_layout()
out = (sys.argv[1].rsplit('.', 1)[0] + '.png') if len(sys.argv) > 1 else 'rms.png'
plt.savefig(out, dpi=120)
print('saved', out)