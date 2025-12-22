from __future__ import annotations
from pathlib import Path
from collections import defaultdict

SNAPSHOT = "cb4b80b"

V_PATH = Path("docs/audit/audit_v0_3_0_v_canon.txt")
E_TARGETS = Path("docs/audit/audit_v0_3_0_edges_targets.tsv")
OUT = Path("docs/audit_v0_3_0_components_by_target.md")

def canon(s: str) -> str:
    s = s.strip()
    if s.startswith("./"):
        s = s[2:]
    return s

def load_v() -> list[str]:
    return [canon(x) for x in V_PATH.read_text(encoding="utf-8").splitlines() if x.strip()]

def load_target_edges() -> list[tuple[str, str, str]]:
    edges: list[tuple[str, str, str]] = []
    for line in E_TARGETS.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        a, b, rel = line.split("\t")
        edges.append((canon(a), canon(b), canon(rel)))
    return edges

def make_component_id(i: int) -> str:
    # A..Z, then A1..Z1, etc. (stable and neutral)
    base = chr(ord("A") + (i % 26))
    suffix = i // 26
    return f"Component-{base}{suffix if suffix else ''}"

def main() -> None:
    V = load_v()
    vset = set(V)
    edges = load_target_edges()

    # file -> targets
    file_targets: dict[str, set[str]] = defaultdict(set)
    for t, f, rel in edges:
        if rel != "TARGET_HAS_SOURCE":
            continue
        if f in vset:
            file_targets[f].add(t)

    # Deterministic single assignment:
    # if file belongs to multiple targets -> lexicographically smallest target wins
    assigned: dict[str, str] = {}
    for f, ts in file_targets.items():
        assigned[f] = sorted(ts)[0]

    # target -> files assigned
    components: dict[str, list[str]] = defaultdict(list)
    for f, t in assigned.items():
        components[t].append(f)

    # Standalone = everything in V not assigned to any target
    assigned_files = set(assigned.keys())
    standalone = sorted([p for p in V if p not in assigned_files])

    # Sort for stability
    for t in components:
        components[t] = sorted(set(components[t]))

    targets_sorted = sorted(components.keys())

    lines: list[str] = []
    lines.append("# ELSIM v0.3.0 — Candidate Components Map (By Build Targets)")
    lines.append("")
    lines.append("Rules of evidence:")
    lines.append("- Source-of-truth artifact set (V): docs/audit/audit_v0_3_0_v_canon.txt")
    lines.append("- Evidence edges: docs/audit/audit_v0_3_0_edges_targets.tsv (TARGET_HAS_SOURCE)")
    lines.append("- Grouping method: each file is assigned to exactly one build target using lexicographic tie-break if multiple targets reference the same file.")
    lines.append(f"- Snapshot: {SNAPSHOT}")
    lines.append("")
    lines.append("## Evidence Artifacts (Edge Lists)")
    lines.append("- docs/audit/audit_v0_3_0_edges_targets.tsv (TARGET_HAS_SOURCE)")
    lines.append("")

    # Components
    for i, t in enumerate(targets_sorted):
        cid = make_component_id(i)
        lines.append(f"## {cid}")
        lines.append("### Included Artifacts")
        for f in components[t]:
            lines.append(f"- {f}")
        lines.append("")
        lines.append("### Basis for Grouping")
        for f in components[t]:
            lines.append(f"- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: {t} -> {f}")
        lines.append("")

    # Standalone (also as Included Artifacts list, so validator can count them)
    lines.append("## Standalone Artifacts")
    lines.append("### Included Artifacts")
    for p in standalone:
        lines.append(f"- {p}")
    lines.append("")
    lines.append("### Basis for Grouping")
    for p in standalone:
        lines.append(f"- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: {p}")
    lines.append("")

    OUT.write_text("\n".join(lines).rstrip() + "\n", encoding="utf-8")

    print(f"Wrote: {OUT}")
    print(f"V: {len(V)}")
    print(f"Targets: {len(targets_sorted)}")
    print(f"Assigned files: {len(assigned_files)}")
    print(f"Standalone: {len(standalone)}")

if __name__ == "__main__":
    main()
