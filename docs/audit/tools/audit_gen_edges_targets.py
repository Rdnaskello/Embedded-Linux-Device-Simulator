from __future__ import annotations
import json
from pathlib import Path

REPO = Path(".")
BUILD = REPO / "build"
REPLY = BUILD / ".cmake" / "api" / "v1" / "reply"

OUT = Path("docs/audit/audit_v0_3_0_edges_targets.tsv")
V_CANON = Path("docs/audit/audit_v0_3_0_v_canon.txt")

def canon(p: str) -> str:
    p = p.strip()
    if p.startswith("./"):
        p = p[2:]
    return p

vset = {canon(x) for x in V_CANON.read_text(encoding="utf-8").splitlines() if x.strip()}

# Find latest index file
idx_files = sorted(REPLY.glob("index-*.json"))
if not idx_files:
    raise SystemExit("No CMake File API reply index-*.json found. Did you run `cmake -S . -B build` after creating the query?")

index = json.loads(idx_files[-1].read_text(encoding="utf-8"))
# Find codemodel object
codemodel_ref = None
for obj in index.get("objects", []):
    if obj.get("kind") == "codemodel" and obj.get("version", {}).get("major") == 2:
        codemodel_ref = obj.get("jsonFile")
        break
if not codemodel_ref:
    raise SystemExit("codemodel-v2 not found in index. Ensure query/codemodel-v2 exists and rerun cmake.")

codemodel = json.loads((REPLY / codemodel_ref).read_text(encoding="utf-8"))

edges: set[tuple[str,str,str]] = set()

# Iterate configurations/targets
for cfg in codemodel.get("configurations", []):
    for tgt in cfg.get("targets", []):
        tgt_json = REPLY / tgt["jsonFile"]
        t = json.loads(tgt_json.read_text(encoding="utf-8"))
        tname = t.get("name", "UNKNOWN_TARGET")
        # collect sources
        for src in t.get("sources", []):
            sp = src.get("path")
            if not sp:
                continue
            # make repo-relative if possible
            try:
                sp_rel = Path(sp)
                if sp_rel.is_absolute():
                    sp_rel = sp_rel.relative_to(REPO.resolve())
                sp_rel = canon(str(sp_rel))
            except Exception:
                sp_rel = canon(sp)
            if sp_rel in vset:
                edges.add((f"cmake-target:{tname}", sp_rel, "TARGET_HAS_SOURCE"))

OUT.write_text("\n".join("\t".join(e) for e in sorted(edges)) + ("\n" if edges else ""), encoding="utf-8")
print(f"Wrote {len(edges)} target edges -> {OUT}")
