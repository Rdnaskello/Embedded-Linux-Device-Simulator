from __future__ import annotations
from collections import defaultdict, deque
from pathlib import Path

SNAPSHOT = "cb4b80b"
V_PATH = Path("/tmp/audit_v_canon.txt")

E_INCLUDE = Path("docs/audit/audit_v0_3_0_edges_includes.tsv")
E_LITERAL = Path("docs/audit/audit_v0_3_0_edges_literals.tsv")
OUT = Path("docs/audit_v0_3_0_components.md")

def load_v() -> list[str]:
    return [x.strip() for x in V_PATH.read_text(encoding="utf-8").splitlines() if x.strip()]

def load_edges(path: Path, vset: set[str], adj: dict[str,set[str]], edges: list[tuple[str,str,str]]):
    if not path.exists():
        return
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        a, b, t = line.split("\t")
        a, b, t = a.strip(), b.strip(), t.strip()
        if a.startswith("./"): a = a[2:]
        if b.startswith("./"): b = b[2:]
        if a in vset and b in vset and a != b:
            edges.append((a, b, t))
            adj[a].add(b)
            adj[b].add(a)

def connected_components(V: list[str], adj: dict[str,set[str]]) -> list[list[str]]:
    seen=set()
    comps=[]
    for v in sorted(V):
        if v in seen:
            continue
        q=deque([v]); seen.add(v)
        comp=[]
        while q:
            x=q.popleft(); comp.append(x)
            for y in adj[x]:
                if y not in seen:
                    seen.add(y); q.append(y)
        comps.append(sorted(comp))
    comps.sort(key=lambda c: c[0])
    return comps

def comp_id(i:int)->str:
    return f"Component-{chr(ord('A')+i)}"

def main():
    V = load_v()
    vset = set(V)

    edges: list[tuple[str,str,str]] = []
    adj: dict[str,set[str]] = defaultdict(set)
    for v in V:
        adj.setdefault(v, set())

    load_edges(E_INCLUDE, vset, adj, edges)
    load_edges(E_LITERAL, vset, adj, edges)

    comps = connected_components(V, adj)
    standalone = [c[0] for c in comps if len(c) == 1 and len(adj[c[0]]) == 0]
    non_standalone = [c for c in comps if not (len(c) == 1 and len(adj[c[0]]) == 0)]

    # evidence spanning tree
    by_pair = defaultdict(list)
    for a,b,t in edges:
        by_pair[(a,b)].append(t)
        by_pair[(b,a)].append(t)

    def pick_type(u,v):
        pref = ["DIRECT_INCLUDE","CO_TEST","CO_EXAMPLE","DIRECT_USE_LITERAL"]
        types = by_pair.get((u,v), [])
        for p in pref:
            if p in types:
                return p
        return types[0] if types else "CONNECTIVITY_EDGE"

    def spanning_tree(comp):
        comp_set=set(comp)
        root=comp[0]
        parent={root:None}
        q=deque([root])
        tree=[]
        while q:
            x=q.popleft()
            for y in sorted(adj[x]):
                if y not in comp_set:
                    continue
                if y in parent:
                    continue
                parent[y]=x
                q.append(y)
                tree.append((x,y,pick_type(x,y)))
        return tree

    lines=[]
    lines += ["# ELSIM v0.3.0 — Candidate Components Map", ""]
    lines += ["## Rules of Evidence",
              "Input source: Audit Index (TASK-AUDIT-0.1). No other sources allowed.",
              "",
              "Snapshot:",
              f"- git: {SNAPSHOT}",
              "",
              "Relationship types used for clustering:",
              "- DIRECT_INCLUDE: extracted from `#include \"...\"` among artifacts in V",
              "- CO_TEST / CO_EXAMPLE / DIRECT_USE_LITERAL: extracted from exact string-literal path references",
              "",
              "Clustering algorithm:",
              "- Undirected graph over V using the relationship types above",
              "- Candidate Components = connected components",
              "- Standalone Artifact = vertex with degree 0",
              ""]
    lines += ["## Inputs used (files)",
              f"- {E_INCLUDE}",
              f"- {E_LITERAL}",
              "",
              "## Input Artifact Set (V)"]
    for v in V:
        lines.append(f"- {v}")
    lines.append("")

    for i, comp in enumerate(non_standalone):
        lines += [f"## {comp_id(i)}", "### Included Artifacts"]
        for p in comp:
            lines.append(f"- {p}")
        lines.append("")
        lines.append("### Basis for Grouping")
        tree = spanning_tree(comp)
        for u,v,t in tree:
            if t == "DIRECT_INCLUDE":
                lines.append(f"- {t}: {u} includes {v}")
            else:
                lines.append(f"- {t}: {u} co-uses {v}")
        lines.append("")

    lines.append("## Standalone Artifacts")
    if not standalone:
        lines.append("- (none)")
    else:
        for p in standalone:
            lines.append(f"### {p}")
            lines.append("Basis: No recorded relationships (degree 0) under the relationship types listed above.")
            lines.append("")

    OUT.write_text("\n".join(lines).rstrip()+"\n", encoding="utf-8")

    print(f"Wrote: {OUT}")
    print(f"V: {len(V)}")
    print(f"E(include): {len(E_INCLUDE.read_text().splitlines()) if E_INCLUDE.exists() else 0}")
    print(f"E(literal): {len(E_LITERAL.read_text().splitlines()) if E_LITERAL.exists() else 0}")
    print(f"Components (non-standalone): {len(non_standalone)}")
    print(f"Standalone: {len(standalone)}")

if __name__ == "__main__":
    main()
