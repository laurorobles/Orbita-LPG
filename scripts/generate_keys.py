import random
import csv

SALT_1 = 0xB12C9F3E48D7A659
SALT_2 = 0x5D8A3C4B9E7F1026

def generate_key():
    val1 = random.randint(1, 0xFFFF)
    
    seed = (val1 << 32) | val1
    
    calc1 = (seed ^ SALT_1) & 0xFFFFFFFFFFFFFFFF
    calc1 = (calc1 * 0x45D9F3B) & 0xFFFFFFFFFFFFFFFF
    expected2 = (calc1 >> 16) & 0xFFFF
    
    seed_shift = ((seed << 13) & 0xFFFFFFFFFFFFFFFF) | (seed >> 19)
    calc2 = (seed_shift ^ SALT_2) & 0xFFFFFFFFFFFFFFFF
    calc2 = (calc2 * 0x27D4EB2D) & 0xFFFFFFFFFFFFFFFF
    expected3 = (calc2 >> 16) & 0xFFFF
    
    expected4 = ((val1 ^ expected2 ^ expected3 ^ 0xBEEF) * 0x119DE1) & 0xFFFF
    
    return f"ORBT-{val1:04X}-{expected2:04X}-{expected3:04X}-{expected4:04X}"

keys = set()
while len(keys) < 500:
    keys.add(generate_key())

with open('orbita_lpg_licenses.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    for k in keys:
        writer.writerow([k])

print("Generated 500 keys in orbita_lpg_licenses.csv")
