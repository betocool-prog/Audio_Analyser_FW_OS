import numpy as np
import pyqtgraph as pg

FS = 96000.0

target_freq=1000.0

N=np.linspace(0, np.uint32(FS / target_freq) - 1, np.uint32(FS / target_freq))

audio_samples = np.sin(2 * np.pi * target_freq * N / FS)

# pg.plot(audio_samples)
# pg.show()

integer_samples=np.uint32(audio_samples * (2**31- 1));

print("{")
for element in integer_samples:
    print("0x{:08X}, ".format(element))
print("}")