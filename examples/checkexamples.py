#!/usr/bin/env python3
"""
checkexamples.py: validate the output of the DGGRID examples on its own
terms, without any reference output.

For every example in the list it checks run health (log, completion
message, error lines, dggrid exit status), that every output file named by
the metafile exists and is non-empty, file syntax (KML, GeoJSON, shapefiles,
text tables), numeric sanity (no nan/inf, lon/lat ranges, closed rings),
grid invariants (whole-earth cell counts, 12 pentagons, neighbour/children
references), and, for whole-earth hexagon grids, the equal-area property.

Prints one PASS/FAIL line per example (with reasons) and exits non-zero on
any FAIL.

Usage (from the examples directory, after doexamples.sh):
   python3 checkexamples.py [--run-log doexamples.log] [--list examples.lst]
                            [--root DIR] [-v] [examples...]

  --run-log FILE  stdout of doexamples.sh; supplies the dggrid exit status
                  of each example. Without it the exit status is not checked.
  --root DIR      check DIR/<ex>/ (e.g. a reference tree such as
                  sampleOutput) instead of <ex>/outputfiles/.
"""

import argparse
import glob
import math
import os
import re
import subprocess
import sys

sys.dont_write_bytecode = True  # keep the examples directory clean
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import exampleslib as L  # noqa: E402

COMPLETION = {
    "GENERATE_GRID": "** grid generation complete **",
    "GENERATE_GRID_FROM_POINTS": "** binning complete **",
    "BIN_POINT_VALS": "** binning complete **",
    "BIN_POINT_PRESENCE": "** binning complete **",
    "TRANSFORM_POINTS": "** transformation complete **",
}
ERROR_RE = re.compile(r"ERROR|fatal|unable to continue|is located on another polygon",
                      re.I)
NANINF_RE = re.compile(r"(?<![A-Za-z0-9_])[-+]?(nan|inf|infinity)(?![A-Za-z0-9_])", re.I)
GENERATED_RE = re.compile(r"^\* generated ([\d,]+) cells")

LOC_EXT = {"KML": ["kml"], "AIGEN": ["gen"], "GEOJSON": ["geojson"],
           "SHAPEFILE": ["shp", "shx", "dbf", "prj"], "TEXT": ["txt"]}

SPHERE_AREA = 4.0 * math.pi


class Result:
    def __init__(self, ex):
        self.ex = ex
        self.fails = []
        self.notes = []

    def fail(self, msg):
        self.fails.append(msg)

    def note(self, msg):
        self.notes.append(msg)


def parse_run_log(path):
    """{example: exit status} from doexamples.sh stdout."""
    status = {}
    cur = None
    with open(path, errors="replace") as f:
        for line in f:
            m = re.match(r"^#### running example (\S+)", line)
            if m:
                cur = m.group(1)
                continue
            m = re.match(r"^dggrid exit status (-?\d+)", line)
            if m and cur is not None:
                status[cur] = int(m.group(1))
            m = re.match(r"^MISSING EXAMPLE (\S+)", line)
            if m:
                status[m.group(1)] = "missing"
    return status


def rel(p):
    """Path for messages: relative to the current directory when inside it."""
    r = os.path.relpath(p)
    return p if r.startswith("..") else r


def out_path(outdir, name):
    """Map a metafile output name (relative to the example dir, normally
    under outputfiles/) into the directory being checked."""
    n = name
    while n.startswith("./"):
        n = n[2:]
    if n.startswith("outputfiles/"):
        n = n[len("outputfiles/"):]
    return os.path.join(outdir, n)


# ---------------------------------------------------------------------------
# expected files
# ---------------------------------------------------------------------------

def expected_outputs(meta, P, nplace):
    """List of (param, base name, kind, extensions) for every output the
    metafile asks for. kind: 'loc' (cell/point/randpts files), 'gdal',
    'collection', 'plain' (single extension-given file), 'exact'
    (output_file_name, never suffixed), 'orient'."""
    def get(k, dflt=None):
        return meta.get(k, P.get(k, dflt))

    outs = []
    op = get("dggrid_operation", "").upper()
    for which in ("cell", "point", "randpts"):
        t = get(which + "_output_type", "NONE").upper()
        if t in ("NONE", ""):
            continue
        if t == "GDAL_COLLECTION":
            continue  # handled below
        name = get(which + "_output_file_name")
        if name is None:
            continue
        if t == "GDAL":
            outs.append((which + "_output_file_name", name, "gdal", [""]))
        elif t in LOC_EXT:
            outs.append((which + "_output_file_name", name, "loc", LOC_EXT[t]))
        else:
            outs.append((which + "_output_file_name", name, "loc", [t.lower()]))
    if any(get(k, "NONE").upper() == "GDAL_COLLECTION" for k in
           ("cell_output_type", "point_output_type", "neighbor_output_type",
            "children_output_type", "indexing_children_output_type",
            "indexing_parent_output_type")):
        outs.append(("collection_output_file_name",
                     get("collection_output_file_name", "cells"), "gdal", [""]))
    for key, ext in (("neighbor", "nbr"), ("children", "chd"),
                     ("indexing_children", "ndxChd"), ("indexing_parent", "ndxPrt")):
        t = get(key + "_output_type", "NONE").upper()
        if t == "TEXT":
            outs.append((key + "_output_file_name",
                         get(key + "_output_file_name"), "loc", [ext]))
    if "output_file_name" in meta and (
            op in ("BIN_POINT_VALS", "BIN_POINT_PRESENCE", "TRANSFORM_POINTS")
            or get("output_file_type", "NONE").upper() != "NONE"):
        outs.append(("output_file_name", meta["output_file_name"], "exact", [""]))
    if nplace > 1:
        outs.append(("dggs_orient_output_file_name",
                     get("dggs_orient_output_file_name", "grid.meta"), "orient", [""]))
    return outs


def find_suffixed(base, ext, placement, maxcells):
    """Files for one output: base[.000N][_k][.ext]."""
    stem = base + (".%04d" % placement if placement else "")
    tail = ("." + ext) if ext else ""
    if maxcells > 0:
        found = []
        k = 1
        while True:
            p = "%s_%d%s" % (stem, k, tail)
            if os.path.exists(p):
                found.append(p)
                k += 1
            else:
                break
        return found
    p = stem + tail
    return [p] if os.path.exists(p) else []


def nonempty(p):
    if os.path.isdir(p):
        fs = [os.path.join(p, x) for x in os.listdir(p) if not x.startswith(".")]
        return bool(fs) and all(os.path.getsize(x) > 0 for x in fs if os.path.isfile(x))
    return os.path.getsize(p) > 0


# ---------------------------------------------------------------------------
# geometry
# ---------------------------------------------------------------------------

def to_vec(lon, lat):
    lo, la = math.radians(lon), math.radians(lat)
    c = math.cos(la)
    return (c * math.cos(lo), c * math.sin(lo), math.sin(la))


def sph_poly_area(ring):
    """Area on the unit sphere of a closed ring (lon/lat degrees) with
    great-circle edges, as a fan of signed triangles from the normalized
    mean vertex (fine for cells, which are small and star-shaped about it)."""
    pts = ring[:-1] if ring[0] == ring[-1] else ring
    vs = [to_vec(lo, la) for lo, la in pts]
    cx = sum(v[0] for v in vs)
    cy = sum(v[1] for v in vs)
    cz = sum(v[2] for v in vs)
    n = math.sqrt(cx * cx + cy * cy + cz * cz)
    c = (cx / n, cy / n, cz / n)
    tot = 0.0
    for i in range(len(vs)):
        a, b = vs[i], vs[(i + 1) % len(vs)]
        # signed triangle (c, a, b) area: Eriksson / van Oosterom-Strackee
        bx = (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
              a[0] * b[1] - a[1] * b[0])
        num = c[0] * bx[0] + c[1] * bx[1] + c[2] * bx[2]
        den = 1.0 + (c[0] * a[0] + c[1] * a[1] + c[2] * a[2]) + \
                    (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]) + \
                    (b[0] * c[0] + b[1] * c[1] + b[2] * c[2])
        tot += 2.0 * math.atan2(num, den)
    return abs(tot)


def sph_perimeter(ring):
    per = 0.0
    for (lo1, la1), (lo2, la2) in zip(ring[:-1], ring[1:]):
        a, b = to_vec(lo1, la1), to_vec(lo2, la2)
        d = math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2)
        per += 2.0 * math.asin(min(1.0, d / 2.0))
    return per


# ---------------------------------------------------------------------------
# per-file checks
# ---------------------------------------------------------------------------

def check_nan_inf(path, res):
    try:
        with open(path, errors="replace") as f:
            for i, line in enumerate(f, 1):
                if NANINF_RE.search(line):
                    res.fail("%s:%d: nan/inf value" % (os.path.basename(path), i))
                    return
    except IsADirectoryError:
        pass


def check_feats_geo(fname, feats, res, lonlim):
    """lon/lat ranges and ring closure for geographic features."""
    nbad = 0
    for ft in feats:
        pts = list(ft["rings"][0]) if ft["rings"] else []
        for r in ft["rings"][1:]:
            pts.extend(r)
        if ft["point"] is not None:
            pts.append(ft["point"])
        for lo, la in pts:
            if not (math.isfinite(lo) and math.isfinite(la)):
                res.fail("%s: cell %s non-finite coordinate" % (fname, ft["id"]))
                return
            if not (-lonlim <= lo <= lonlim and -90.0 <= la <= 90.0):
                nbad += 1
                if nbad == 1:
                    res.fail("%s: cell %s coordinate out of range (%r, %r)"
                             % (fname, ft["id"], lo, la))
        if ft["kind"] == "polygon":
            if not ft["rings"]:
                res.fail("%s: cell %s polygon without ring" % (fname, ft["id"]))
            for r in ft["rings"]:
                if len(r) < 4:
                    res.fail("%s: cell %s ring has %d < 4 points"
                             % (fname, ft["id"], len(r)))
                    return
                if r[0] != r[-1]:
                    res.fail("%s: cell %s ring not closed" % (fname, ft["id"]))
                    return


def read_features(path, res):
    """Read a location file into features, checking syntax. Returns
    (features, format) or (None, None) if the format is not geographic."""
    fname = rel(path)
    ext = path.rsplit(".", 1)[-1].lower() if "." in os.path.basename(path) else ""
    try:
        if ext == "kml":
            if L.have_tool("xmllint"):
                r = subprocess.run(["xmllint", "--noout", path],
                                   capture_output=True, text=True)
                if r.returncode != 0:
                    res.fail("%s: xmllint: %s" % (fname, r.stderr.strip()[:200]))
            return L.read_kml(path), "kml"
        if ext == "geojson" or ext == "json":
            feats, bad = L.read_geojson(path)
            for b in bad[:3]:
                res.fail("%s: %s" % (fname, b))
            return feats, "geojson"
        if ext == "gen":
            feats, bad = L.read_aigen(path)
            for b in bad[:3]:
                res.fail("%s: %s" % (fname, b))
            return feats, "gen"
        if ext == "shp":
            n, err = L.ogrinfo_feature_count(path)
            if n is None:
                res.fail("%s: ogrinfo -so failed: %s" % (fname, err))
                return None, None
            feats, bad = L.read_shapefile(path)
            for b in bad[:3]:
                res.fail("%s: %s" % (fname, b))
            if len(feats) != n:
                res.fail("%s: ogrinfo reports %d features but dump has %d"
                         % (fname, n, len(feats)))
            res.note("%s: %d features" % (os.path.basename(path), n))
            return feats, "shp"
    except Exception as e:  # syntax errors surface here
        res.fail("%s: cannot parse: %s" % (fname, e))
        return None, None
    return None, None


def split_cols(line, delim):
    if delim == " ":
        return line.split()
    return [c for part in line.rstrip("\n").split(delim) for c in [part]]


def check_table(path, res, delim, var_cols=False):
    """Consistent column count; returns rows (lists of columns)."""
    rows = []
    with open(path, errors="replace") as f:
        for line in f:
            if not line.strip():
                continue
            rows.append(line.split() if delim == " " else
                        re.split(r"[%s\s]+" % re.escape(delim), line.strip()))
    if not var_cols:
        counts = {}
        for r in rows:
            counts[len(r)] = counts.get(len(r), 0) + 1
        if len(counts) > 1:
            res.fail("%s: inconsistent column counts %s"
                     % (os.path.basename(path), dict(sorted(counts.items()))))
    return rows


# ---------------------------------------------------------------------------
# per-example check
# ---------------------------------------------------------------------------

def check_example(ex, args, run_status):
    res = Result(ex)
    exdir = os.path.join(L.EXAMPLES_DIR, ex)
    metafile = os.path.join(exdir, ex + ".meta")
    if not os.path.isfile(metafile):
        res.fail("missing metafile %s" % metafile)
        return res
    outdir = os.path.join(args.root, ex) if args.root else os.path.join(exdir, "outputfiles")
    if not os.path.isdir(outdir):
        res.fail("missing output directory %s" % outdir)
        return res
    meta = L.parse_metafile(metafile)

    # ---- run health
    logf = os.path.join(outdir, ex + ".txt")
    log_lines = []
    if not os.path.isfile(logf):
        res.fail("missing log %s" % logf)
    else:
        with open(logf, errors="replace") as f:
            log_lines = [l.rstrip("\n") for l in f]
        for i, line in enumerate(log_lines, 1):
            if ERROR_RE.search(line):
                res.fail("log line %d: %s" % (i, line.strip()[:120]))
                break
    blocks = L.parse_log_params(log_lines)
    P = blocks[0] if blocks else {}
    op = meta.get("dggrid_operation", P.get("dggrid_operation", "")).upper()
    if log_lines:
        tail = [l for l in log_lines if l.strip()]
        if op in COMPLETION:
            if COMPLETION[op] not in (l.strip() for l in log_lines):
                res.fail("log lacks completion message '%s'" % COMPLETION[op])
        elif op == "OUTPUT_STATS":
            # the table ends with the row for the last resolution
            rs = P.get("dggs_res_spec", meta.get("dggs_res_spec", "")).strip()
            if not tail or tail[-1].split()[0] != rs:
                res.fail("OUTPUT_STATS table does not end at resolution %s" % rs)
        else:
            res.fail("unknown operation %s" % op)
    if run_status is not None:
        st = run_status.get(ex)
        if st is None:
            res.fail("no dggrid exit status in run log")
        elif st != 0:
            res.fail("dggrid exit status %s" % st)
    else:
        res.note("exit status not checked (no --run-log)")

    # ---- expected files
    nplace = int(meta.get("dggs_num_placements", P.get("dggs_num_placements", "1")))
    maxcells = int(meta.get("max_cells_per_output_file",
                            P.get("max_cells_per_output_file", "0")))
    placements = list(range(1, nplace + 1)) if nplace > 1 else [0]
    delim = meta.get("output_delimiter", P.get("output_delimiter", " "))
    wrap = meta.get("longitude_wrap_mode", P.get("longitude_wrap_mode", "WRAP")).upper()
    lonlim = 180.0 if wrap == "WRAP" else 360.0
    loc_files = {}      # placement -> list of (param, path)
    other_files = {}    # placement -> list of (param, path, ext)
    expected_paths = set()
    for param, base, kind, exts in expected_outputs(meta, P, nplace):
        b = out_path(outdir, base)
        if kind == "exact":
            ps = [b] if os.path.exists(b) else []
            if not ps:
                res.fail("missing output %s (%s)" % (rel(b), param))
            for p in ps:
                expected_paths.add(os.path.abspath(p))
                other_files.setdefault(0, []).append((param, p, "table"))
            continue
        if kind == "orient":
            for pl in placements:
                p = "%s.%04d" % (b, pl)
                if not os.path.exists(p):
                    res.fail("missing orientation file %s" % rel(p))
                else:
                    expected_paths.add(os.path.abspath(p))
            continue
        for pl in placements:
            for ext in exts:
                if kind == "gdal":
                    ps = find_suffixed(b, "", pl, maxcells)
                else:
                    ps = find_suffixed(b, ext, pl, maxcells)
                if not ps:
                    if ext == "prj":
                        continue  # optional for shapefiles
                    res.fail("missing output %s%s%s (%s)" % (
                        rel(b), ".%04d" % pl if pl else "",
                        "." + ext if ext else "", param))
                    continue
                for p in ps:
                    expected_paths.add(os.path.abspath(p))
                    if not nonempty(p):
                        res.fail("empty output %s" % rel(p))
                        continue
                    if kind == "gdal" and os.path.isdir(p):
                        for q in sorted(glob.glob(os.path.join(p, "*"))):
                            expected_paths.add(os.path.abspath(q))
                            if q.endswith(".shp"):
                                loc_files.setdefault(pl, []).append((param, q))
                    elif ext in ("kml", "gen", "geojson", "shp", ""):
                        loc_files.setdefault(pl, []).append((param, p))
                    elif ext in ("nbr", "chd", "ndxChd", "ndxPrt", "txt"):
                        other_files.setdefault(pl, []).append((param, p, ext))
    # files present that the metafile does not explain
    for rootd, _, files in os.walk(outdir):
        for fn in files:
            p = os.path.abspath(os.path.join(rootd, fn))
            if fn.startswith(".") or p == os.path.abspath(logf):
                continue
            if p not in expected_paths:
                res.note("unexpected file %s" % os.path.relpath(p, outdir))

    # ---- nan/inf over every text output
    for rootd, _, files in os.walk(outdir):
        for fn in files:
            if fn.startswith(".") or fn.rsplit(".", 1)[-1] in ("shp", "shx", "dbf"):
                continue
            check_nan_inf(os.path.join(rootd, fn), res)

    # ---- per-placement: syntax, numeric sanity, grid invariants
    generated = []
    for line in log_lines:
        m = GENERATED_RE.match(line)
        if m:
            generated.append(int(m.group(1).replace(",", "")))
    whole = (op == "GENERATE_GRID" and
             P.get("clip_subset_type", meta.get("clip_subset_type", "WHOLE_EARTH")).upper()
             == "WHOLE_EARTH")
    label_seq = (P.get("output_cell_label_type", "GLOBAL_SEQUENCE").upper() == "GLOBAL_SEQUENCE"
                 or P.get("output_address_type", "SEQNUM").upper() == "SEQNUM")
    try:
        res_spec = int(P.get("dggs_res_spec", meta.get("dggs_res_spec", "")).split()[0])
    except (ValueError, IndexError):
        res_spec = None

    for pi, pl in enumerate(placements):
        PP = blocks[pi + 1] if (nplace > 1 and len(blocks) > pi + 1) else P
        cells = {}        # id -> feature (polygon), from cell outputs
        points = {}       # id -> feature, from point outputs
        coll_nbrs = {}    # from GDAL collections
        coll_chds = {}
        for param, p in loc_files.get(pl, []):
            feats, fmt = read_features(p, res)
            if feats is None:
                continue
            check_feats_geo(os.path.relpath(p, outdir), feats, res, lonlim)
            for ft in feats:
                if ft["kind"] == "polygon" and not param.startswith("point"):
                    if ft["id"] in cells and op == "GENERATE_GRID":
                        res.fail("%s: duplicate cell %s" % (os.path.basename(p), ft["id"]))
                    cells[ft["id"]] = ft
                if ft["point"] is not None and (param.startswith("point") or
                                                param.startswith("collection")):
                    points[ft["id"]] = ft
                if "neighbors" in ft["props"]:
                    coll_nbrs[ft["id"]] = [str(x) for x in ft["props"]["neighbors"]]
                if "children" in ft["props"]:
                    coll_chds[ft["id"]] = [str(x) for x in ft["props"]["children"]]
        ids = set(cells) | set(points)

        # text tables
        tables = {}
        for param, p, ext in other_files.get(pl, []):
            rows = check_table(p, res, delim if ext in ("table", "txt") else " ",
                               var_cols=(ext in ("nbr", "chd", "ndxChd")
                                         or op == "TRANSFORM_POINTS"))
            tables[ext] = (p, rows)

        # cell counts
        if whole and ids:
            n = len(ids)
            if generated:
                g = generated[pi] if pi < len(generated) else generated[-1]
                if g != n:
                    res.fail("%s%d cells in output but dggrid printed %d"
                             % (("placement %d: " % pl) if pl else "", n, g))
            r = res_spec
            if r is None:  # e.g. CELL_AREA resolution: find it
                for rr in range(0, 40):
                    if L.whole_earth_cell_count(PP, rr) == n:
                        r = rr
                        break
            expect = L.whole_earth_cell_count(PP, r) if r is not None else None
            if expect is None:
                res.fail("cannot compute whole-earth cell count")
            elif expect != n:
                res.fail("whole-earth res %s: %d cells, expected %d" % (r, n, expect))
            else:
                res.note("whole-earth res %d: %d cells OK" % (r, n))

        # pentagons and equal area (hexagon grids)
        topo = PP.get("dggs_topology", "HEXAGON").upper()
        dens = int(PP.get("densification", meta.get("densification", "0")))
        prec = int(PP.get("precision", meta.get("precision", "7")))
        if whole and topo == "HEXAGON" and cells:
            nc = {}
            corners = {}
            for cid, ft in cells.items():
                npt = len(ft["rings"][0]) - 1
                k = npt / (dens + 1)
                corners[cid] = k
                nc[k] = nc.get(k, 0) + 1
            if set(nc) - {5.0, 6.0}:
                res.fail("hexagon grid: unexpected vertex counts %s" % nc)
            elif nc.get(5.0, 0) != 12:
                res.fail("hexagon grid: %d pentagons, expected 12" % nc.get(5.0, 0))
            else:
                res.note("12 pentagons OK")
            if len(cells) == len(ids) and len(cells) > 12:
                area_check(res, cells, corners, prec, dens, pl)

        # neighbours and children
        n_r = L.whole_earth_cell_count(PP, res_spec) if res_spec is not None else None
        n_r1 = L.whole_earth_cell_count(PP, res_spec + 1) if res_spec is not None else None
        nbrs = dict(coll_nbrs)
        chds = dict(coll_chds)
        if "nbr" in tables:
            for row in tables["nbr"][1]:
                nbrs[row[0]] = row[1:]
        if "chd" in tables:
            for row in tables["chd"][1]:
                chds[row[0]] = row[1:]
        for ext in ("ndxChd", "ndxPrt"):
            if ext in tables and ids:
                missing = [row[0] for row in tables[ext][1] if row[0] not in ids]
                if missing:
                    res.fail("%s references %d cells not in the grid output, e.g. %s"
                             % (ext, len(missing), missing[0]))
        check_refs(res, "neighbour", nbrs, ids, whole, label_seq, n_r)
        check_refs(res, "children", chds, ids, False, label_seq, n_r1)
    return res


def check_refs(res, what, table, ids, whole, label_seq, nmax):
    if not table:
        return
    bad_src = [c for c in table if ids and c not in ids]
    if bad_src:
        res.fail("%s list for %d cells not in the grid output, e.g. %s"
                 % (what, len(bad_src), bad_src[0]))
    nbad = 0
    ex = None
    for c, refs in table.items():
        for x in refs:
            ok = True
            if whole and ids:
                ok = x in ids
            elif label_seq and nmax is not None:
                ok = x.isdigit() and 1 <= int(x) <= nmax
            if not ok:
                nbad += 1
                ex = ex or (c, x)
    if nbad:
        res.fail("%s: %d references to non-existent cells, e.g. %s -> %s"
                 % (what, nbad, ex[0], ex[1]))
    else:
        res.note("%s references OK (%d cells)" % (what, len(table)))


# Allowance for approximating the true (curved) cell edges by great-circle
# chords between the output vertices, relative to the cell area, per
# edge subdivision: GC_EDGE_K / (densification+1)^2. Measured for ISEA on
# this suite: worst 0.153 (pentagons, densification 0), 0.116 (7H, 0),
# 0.0029 (3H res 5, densification 3), with the expected 1/(d+1)^2 decay
# (3H res 4: 0.0378, 0.0027, 3.6e-4, 4.6e-5 at d = 0, 3, 10, 30).
GC_EDGE_K = 0.25
# Allowance on the total 4*pi*R^2: the great-circle polygons do not tile
# the sphere exactly where vertices of neighbouring cells are not shared
# (measured 2.9e-6 for 7H at res 3, independent of densification).
GC_TOTAL_TOL = 1.0e-5


def area_check(res, cells, corners, prec, dens, pl):
    """Equal-area check for a whole-earth hexagon grid on the unit sphere.

    Every hexagon has area A = 4*pi/(N-2) and every pentagon 5/6 A (the 12
    pentagons each lack one of the six wedges). Cell areas are computed as
    spherical polygons with great-circle edges between the output vertices.

    Tolerance per cell = printed-precision bound + great-circle-edge
    allowance:
      - The coordinates are printed with `prec` decimals, so each vertex
        may be off by up to 0.5*10^-prec degrees in lon and lat; moving
        the vertices of a ring by at most delta = sqrt(2)*0.5*10^-prec
        (radians) changes its area by at most about delta*perimeter.
      - The true cell edges are images of straight planar segments, not
        great circles; the chord error is GC_EDGE_K/(dens+1)^2 relative.
    The total must equal 4*pi within the summed printed-precision bounds
    plus GC_TOTAL_TOL (relative).

    The worst relative deviation reported is the raw great-circle-polygon
    figure, so it can be compared between runs (it is dominated by the
    edge approximation, not by the projection, at low densification).
    """
    n = len(cells)
    a_hex = SPHERE_AREA / (n - 2)
    delta = math.sqrt(2.0) * 0.5 * 10.0 ** (-prec) * math.pi / 180.0
    gc_rel = GC_EDGE_K / (dens + 1) ** 2
    worst = 0.0
    worst_id = None
    total = 0.0
    tot_tol = 0.0
    nfail = 0
    first_fail = None
    for cid, ft in cells.items():
        ring = ft["rings"][0]
        a = sph_poly_area(ring)
        total += a
        ptol = delta * sph_perimeter(ring)
        tot_tol += ptol
        expect = a_hex * (5.0 / 6.0 if corners[cid] == 5.0 else 1.0)
        dev = abs(a - expect) / expect
        if dev > worst:
            worst, worst_id = dev, cid
        if abs(a - expect) > ptol + gc_rel * expect:
            nfail += 1
            first_fail = first_fail or (cid, dev, ptol / expect + gc_rel)
    tag = ("placement %d: " % pl) if pl else ""
    tot_dev = (total - SPHERE_AREA) / SPHERE_AREA
    tot_allow = tot_tol / SPHERE_AREA + GC_TOTAL_TOL
    msg = ("%sequal area: worst rel. deviation %.3e (cell %s; allowed %.1e + "
           "precision); sum vs 4piR^2 %+.3e (allowed %.1e)"
           % (tag, worst, worst_id, gc_rel, tot_dev, tot_allow))
    res.note(msg)
    res.area = getattr(res, "area", []) + [(worst, worst_id, tot_dev)]
    if nfail:
        res.fail("%s%d cells deviate from equal area beyond tolerance, first "
                 "cell %s: %.3e > %.3e" % (tag, nfail, first_fail[0],
                                           first_fail[1], first_fail[2]))
    if abs(tot_dev) > tot_allow:
        res.fail("%scell areas sum to 4piR^2 %+.3e rel., beyond tolerance %.3e"
                 % (tag, tot_dev, tot_allow))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[1],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("examples", nargs="*", help="examples to check (default: all in list)")
    ap.add_argument("--list", default=os.path.join(L.EXAMPLES_DIR, "examples.lst"))
    ap.add_argument("--run-log", help="doexamples.sh stdout (for dggrid exit status)")
    ap.add_argument("--root", help="check DIR/<ex>/ instead of <ex>/outputfiles/")
    ap.add_argument("-v", "--verbose", action="store_true", help="print notes too")
    args = ap.parse_args()
    if args.root:
        args.root = os.path.abspath(args.root)
    exs = args.examples or L.read_example_list(args.list)
    run_status = parse_run_log(args.run_log) if args.run_log else None
    if not L.have_tool("ogrinfo"):
        print("NOTE: ogrinfo not found; shapefiles cannot be checked")
    if not L.have_tool("xmllint"):
        print("NOTE: xmllint not found; KML well-formedness not checked")
    nfail = 0
    areas = []
    for ex in exs:
        res = check_example(ex, args, run_status)
        if res.fails:
            nfail += 1
            print("FAIL %s: %s" % (ex, "; ".join(res.fails[:8]) +
                                   (" (+%d more)" % (len(res.fails) - 8)
                                    if len(res.fails) > 8 else "")))
        else:
            print("PASS %s" % ex)
        for a in getattr(res, "area", []):
            areas.append((a[0], ex, a[1], a[2]))
        for n in res.notes:
            if args.verbose or "equal area" in n:
                print("     %s" % n)
    if areas:
        w = max(areas)
        print("equal-area worst relative deviation: %.3e (%s, cell %s)" % (w[0], w[1], w[2]))
    print("checkexamples: %d PASS / %d FAIL" % (len(exs) - nfail, nfail))
    sys.exit(1 if nfail else 0)


if __name__ == "__main__":
    main()
