import os
import shutil

base = r"c:\Users\knith\Documents\My Projects\SchedTiny\firmware"
src = os.path.join(base, "src")
core = os.path.join(base, "core")

if not os.path.exists(core):
    os.makedirs(core)

# Move hal to drivers
hal = os.path.join(src, "hal")
drivers = os.path.join(core, "drivers")
if os.path.exists(hal):
    os.rename(hal, drivers)

# Move bench to core/bench
bench = os.path.join(src, "bench")
if os.path.exists(bench):
    os.rename(bench, os.path.join(core, "bench"))

# Move scheduler to core/scheduler
scheduler = os.path.join(src, "scheduler")
if os.path.exists(scheduler):
    os.rename(scheduler, os.path.join(core, "scheduler"))

# Move ml to core/ml
ml = os.path.join(src, "ml")
if os.path.exists(ml):
    os.rename(ml, os.path.join(core, "ml"))
    
# Clean up src if empty
if os.path.exists(src) and not os.listdir(src):
    os.rmdir(src)

print("Moved folders.")
