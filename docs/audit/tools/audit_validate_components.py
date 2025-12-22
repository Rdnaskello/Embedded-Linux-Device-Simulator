from __future__ import annotations
from pathlib import Path
from collections import Counter

V_PATH = Path("/tmp/audit_v_canon.txt")
DOC_PATH = Path("docs/audit_v0_3_0_components.md")

def canon(s: str) -> str:
    s = s.strip()
    if s.startswith("./"):
        s = s[2:]
    return s

V = [canon(x) for x in V_PATH.read_text(encoding="utf-8").splitlines() if x.strip()]
vset = set(V)

lines = DOC_PATH.read_text(encoding="utf-8").splitlines()

in_component = False
in_included = False
in_standalone = False

mapped = []

for line in lines:
    # Section headers
    if line.startswith("## "):
        title = line[3:].strip()
        in_component = title.startswith("Component-")
        in_standalone = (title == "Standalone Artifacts")
        in_included = False
        continue

    # Subsection inside component
    if in_component and line.startswith("### "):
        sub = line[4:].strip()
        in_included = (sub == "Included Artifacts")
        continue

    # Standalone artifact headings
    if in_standalone and line.startswith("### "):
        p = canon(line[4:])
        mapped.append(p)
        continue

    # Included Artifacts bullets
    if in_component and in_included and line.startswith("- "):
        p = canon(line[2:])
        mapped.append(p)
        continue

# Filter only paths that are actually in V
mapped_in_v = [p for p in mapped if p in vset]
cnt = Counter(mapped_in_v)

missing = [p for p in V if cnt[p] == 0]
dupes = [p for p,c in cnt.items() if c > 1]

print("V size:", len(V))
print("Mapped paths (in V):", len(mapped_in_v))
print("Missing:", len(missing))
print("Duplicates:", len(dupes))

if missing:
    print("First missing examples:", missing[:15])
if dupes:
    print("Duplicate examples:", dupes[:15])
