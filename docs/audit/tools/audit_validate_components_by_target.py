from __future__ import annotations
from pathlib import Path
from collections import Counter

V_PATH = Path("docs/audit/audit_v0_3_0_v_canon.txt")
DOC_PATH = Path("docs/audit_v0_3_0_components_by_target.md")

def canon(s: str) -> str:
    s = s.strip()
    if s.startswith("./"):
        s = s[2:]
    return s

V = [canon(x) for x in V_PATH.read_text(encoding="utf-8").splitlines() if x.strip()]
vset = set(V)

lines = DOC_PATH.read_text(encoding="utf-8").splitlines()

# We count bullet paths under:
# - any "### Included Artifacts" block (both for components and for Standalone Artifacts section)
in_included = False
mapped: list[str] = []

for line in lines:
    if line.strip() == "### Included Artifacts":
        in_included = True
        continue
    if line.startswith("### ") and line.strip() != "### Included Artifacts":
        in_included = False
        continue
    if line.startswith("## "):  # new section
        in_included = False
        continue

    if in_included and line.startswith("- "):
        p = canon(line[2:])
        if p in vset:
            mapped.append(p)

cnt = Counter(mapped)
missing = [p for p in V if cnt[p] == 0]
dupes = [p for p, c in cnt.items() if c > 1]

print("V size:", len(V))
print("Mapped paths (in V):", len(mapped))
print("Missing:", len(missing))
print("Duplicates:", len(dupes))
if missing:
    print("First missing examples:", missing[:10])
if dupes:
    print("Duplicate examples:", dupes[:10])
