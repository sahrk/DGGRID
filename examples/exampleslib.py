"""
exampleslib.py: helpers shared by checkexamples.py and cmpexamples.py

Parsing of DGGRID metafiles and logs, and readers for the output formats
the examples produce (KML, GeoJSON, AIGEN, shapefiles via ogrinfo, dBase).
Standard library only, plus the external tools ogrinfo and xmllint.
"""

import json
import os
import re
import shutil
import subprocess

EXAMPLES_DIR = os.path.dirname(os.path.abspath(__file__))

# ---------------------------------------------------------------------------
# example lists and metafiles
# ---------------------------------------------------------------------------

def read_example_list(path):
    """Whitespace-separated example names, as `cat examples.lst` gives them."""
    with open(path) as f:
        return f.read().split()


def parse_metafile(path):
    """Parse a metafile with the same rules as DgApParamList::loadParams().

    A line of length <= 1 or starting with '#' is skipped. The name is the
    first whitespace-delimited token (compared case-insensitively); the value
    is the rest of the line with leading whitespace removed. We also strip
    trailing whitespace, and one pair of enclosing double quotes (as the
    string parameters do). A value of "invalid" is ignored, as in setParam().
    Later settings override earlier ones.
    """
    params = {}
    with open(path, errors="replace") as f:
        for line in f:
            line = line.rstrip("\n")
            if len(line) <= 1 or line[0] == "#":
                continue
            s = line.lstrip()
            if not s:
                continue
            parts = s.split(None, 1)
            name = parts[0].lower()
            val = parts[1].strip() if len(parts) > 1 else ""
            if len(val) >= 2 and val[0] == '"' and val[-1] == '"':
                val = val[1:-1]
            if val.lower() == "invalid":
                continue
            params[name] = val
    return params


_ECHO_RE = re.compile(r"^(\S+) (.*) \((user set|default)\)$")


def parse_log_params(log_lines):
    """Parameter echo blocks from a dggrid log.

    Returns a list of dicts, one per '* parameter values:' block (the first
    is the global list; with dggs_num_placements > 1 each placement echoes
    its own list after it).
    """
    blocks = []
    cur = None
    for line in log_lines:
        m = _ECHO_RE.match(line)
        if m:
            if cur is None:
                cur = {}
                blocks.append(cur)
            val = m.group(2)
            if len(val) >= 2 and val[0] == '"' and val[-1] == '"':
                val = val[1:-1]
            if m.group(1) in cur:  # a new block started without a gap
                cur = {}
                blocks.append(cur)
            cur[m.group(1)] = val
        else:
            cur = None
    return blocks


# ---------------------------------------------------------------------------
# grid arithmetic
# ---------------------------------------------------------------------------

def aperture_sequence(p):
    """Per-resolution apertures (resolution 1, 2, ...) from resolved params,
    or None if unknown."""
    atype = p.get("dggs_aperture_type", "PURE").upper()
    if atype == "PURE":
        a = int(p.get("dggs_aperture", "4"))
        return lambda i: a
    if atype == "MIXED43":
        n4 = int(p.get("dggs_num_aperture_4_res", "0"))
        return lambda i: 4 if i <= n4 else 3
    if atype == "SEQUENCE":
        seq = p.get("dggs_aperture_sequence", "")
        return lambda i: int(seq[i - 1]) if i - 1 < len(seq) else None
    return None


def whole_earth_cell_count(p, r):
    """Number of cells in a whole-earth grid at resolution r, or None."""
    ap = aperture_sequence(p)
    if ap is None:
        return None
    prod = 1
    for i in range(1, r + 1):
        a = ap(i)
        if a is None:
            return None
        prod *= a
    topo = p.get("dggs_topology", "HEXAGON").upper()
    if topo == "HEXAGON":
        return 10 * prod + 2
    if topo == "DIAMOND":
        return 10 * prod
    if topo == "TRIANGLE":
        return 20 * prod
    return None


# ---------------------------------------------------------------------------
# readers. Each returns a list of "features":
#   {"id": str|None, "kind": "polygon"|"point"|"other",
#    "rings": [[(lon, lat), ...], ...], "point": (lon, lat)|None,
#    "props": dict}
# ---------------------------------------------------------------------------

_KML_PM_RE = re.compile(r"<Placemark>(.*?)</Placemark>", re.S)
_KML_NAME_RE = re.compile(r"<name>(.*?)</name>", re.S)
_KML_COORD_RE = re.compile(r"<coordinates>(.*?)</coordinates>", re.S)


def _parse_coord_pairs(text):
    pts = []
    for tok in text.split():
        xy = tok.split(",")
        pts.append((float(xy[0]), float(xy[1])))
    return pts


def read_kml(path):
    with open(path, errors="replace") as f:
        txt = f.read()
    feats = []
    for m in _KML_PM_RE.finditer(txt):
        body = m.group(1)
        nm = _KML_NAME_RE.search(body)
        cid = nm.group(1).strip() if nm else None
        coords = [_parse_coord_pairs(c) for c in _KML_COORD_RE.findall(body)]
        if "<Point>" in body:
            feats.append({"id": cid, "kind": "point", "rings": [],
                          "point": coords[0][0] if coords and coords[0] else None,
                          "props": {}})
        elif "<LineString>" in body or "<Polygon>" in body or "<LinearRing>" in body:
            feats.append({"id": cid, "kind": "polygon", "rings": coords,
                          "point": None, "props": {}})
        else:
            feats.append({"id": cid, "kind": "other", "rings": [],
                          "point": None, "props": {}})
    return feats


def _geojson_geoms(g, out, bad):
    if g is None:
        bad.append("null geometry")
        return
    t = g.get("type")
    if t == "Polygon":
        out.append(("polygon", [[(float(x[0]), float(x[1])) for x in ring]
                                for ring in g["coordinates"]]))
    elif t == "Point":
        c = g["coordinates"]
        out.append(("point", (float(c[0]), float(c[1]))))
    elif t == "GeometryCollection":
        for sub in g.get("geometries", []):
            _geojson_geoms(sub, out, bad)
    else:
        bad.append("unexpected geometry type %s" % t)


def read_geojson(path):
    """Returns (features, problems)."""
    with open(path, errors="replace") as f:
        data = json.load(f)
    feats, bad = [], []
    if data.get("type") != "FeatureCollection":
        bad.append("not a FeatureCollection")
    for ft in data.get("features", []):
        props = ft.get("properties") or {}
        cid = props.get("name")
        cid = str(cid) if cid is not None else None
        geoms = []
        _geojson_geoms(ft.get("geometry"), geoms, bad)
        polys = [g for k, g in geoms if k == "polygon"]
        pts = [g for k, g in geoms if k == "point"]
        rings = [r for p in polys for r in p]
        kind = "polygon" if polys else ("point" if pts else "other")
        feats.append({"id": cid, "kind": kind, "rings": rings,
                      "point": pts[0] if pts else None, "props": props})
    return feats, bad


def read_aigen(path):
    """AIGEN (.gen): 'id x y' header (cell centre or point), then for a cell
    the boundary vertices one per line, each record ending with END; the
    file ends with an extra END."""
    feats, bad = [], []
    with open(path, errors="replace") as f:
        lines = [l.split() for l in f if l.strip()]
    i, n = 0, len(lines)
    while i < n:
        t = lines[i]
        if t == ["END"]:
            i += 1
            continue
        if len(t) != 3:
            bad.append("line %d: expected 'id x y', got %r" % (i + 1, " ".join(t)))
            i += 1
            continue
        cid = t[0]
        ctr = (float(t[1]), float(t[2]))
        i += 1
        ring = []
        while i < n and lines[i] != ["END"] and len(lines[i]) == 2:
            ring.append((float(lines[i][0]), float(lines[i][1])))
            i += 1
        if i < n and lines[i] == ["END"]:
            i += 1
            feats.append({"id": cid, "kind": "polygon" if ring else "point",
                          "rings": [ring] if ring else [], "point": ctr,
                          "props": {}})
        else:
            # a point file: records are 'id x y' lines, terminated by END
            feats.append({"id": cid, "kind": "point", "rings": [],
                          "point": ctr, "props": {}})
    return feats, bad


def have_tool(name):
    return shutil.which(name) is not None


def ogrinfo_dump(path):
    """`ogrinfo -al -q` text dump of a vector file, run from the file's
    directory so that paths do not appear. Metadata lines that hold the
    creation date (DBF_DATE_LAST_UPDATE) are removed."""
    d, b = os.path.split(os.path.abspath(path))
    r = subprocess.run(["ogrinfo", "-al", "-q", b], cwd=d,
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError("ogrinfo failed on %s: %s" % (path, r.stderr.strip()))
    out = []
    for line in r.stdout.splitlines():
        if "DBF_DATE_LAST_UPDATE" in line:
            continue
        out.append(line)
    # drop now-empty "Metadata:" headers
    res = []
    for i, line in enumerate(out):
        if line.strip() == "Metadata:" and (i + 1 >= len(out) or
                                             not out[i + 1].startswith("  ")
                                             or "=" not in out[i + 1]):
            continue
        res.append(line)
    return res


def ogrinfo_feature_count(path):
    d, b = os.path.split(os.path.abspath(path))
    r = subprocess.run(["ogrinfo", "-al", "-so", b], cwd=d,
                       capture_output=True, text=True)
    if r.returncode != 0:
        return None, r.stderr.strip()
    m = re.search(r"Feature Count: (\d+)", r.stdout)
    return (int(m.group(1)) if m else None), None


_WKT_RE = re.compile(r"^\s*(POLYGON|POINT|MULTIPOLYGON|LINESTRING)\s*(.*)$")
_FIELD_RE = re.compile(r"^\s+(\w+) \((\w+)\) = (.*)$")


def read_shapefile(path):
    """Features from a shapefile via its ogrinfo dump."""
    feats, bad = [], []
    cur = None
    for line in ogrinfo_dump(path):
        if line.startswith("OGRFeature("):
            cur = {"id": None, "kind": "other", "rings": [], "point": None,
                   "props": {}}
            feats.append(cur)
            continue
        if cur is None:
            continue
        m = _FIELD_RE.match(line)
        if m:
            cur["props"][m.group(1)] = m.group(3)
            if m.group(1) in ("name", "global_id") and cur["id"] is None:
                cur["id"] = m.group(3).strip()
            continue
        m = _WKT_RE.match(line)
        if m:
            kind, rest = m.group(1), m.group(2)
            if kind == "POINT":
                xy = rest.strip("() ").split()
                cur["kind"] = "point"
                cur["point"] = (float(xy[0]), float(xy[1]))
            elif kind == "POLYGON":
                cur["kind"] = "polygon"
                for ring in re.findall(r"\(([^()]*)\)", rest):
                    pts = []
                    for pr in ring.split(","):
                        xy = pr.split()
                        pts.append((float(xy[0]), float(xy[1])))
                    cur["rings"].append(pts)
            else:
                bad.append("unexpected geometry %s" % kind)
    return feats, bad


def read_dbf_records(path):
    """Minimal dBase III reader: (field names, list of record tuples).
    The header's last-update date is ignored."""
    with open(path, "rb") as f:
        data = f.read()
    nrec = int.from_bytes(data[4:8], "little")
    hlen = int.from_bytes(data[8:10], "little")
    rlen = int.from_bytes(data[10:12], "little")
    fields = []
    pos = 32
    while pos < hlen - 1 and data[pos] != 0x0D:
        name = data[pos:pos + 11].split(b"\0")[0].decode("latin-1")
        ftype = chr(data[pos + 11])
        flen = data[pos + 16]
        fields.append((name, ftype, flen))
        pos += 32
    recs = []
    for i in range(nrec):
        off = hlen + i * rlen
        rec = data[off:off + rlen]
        vals = []
        p = 1  # deletion flag
        for (_, _, flen) in fields:
            vals.append(rec[p:p + flen].decode("latin-1").strip())
            p += flen
        recs.append((rec[:1].decode("latin-1"), tuple(vals)))
    return [f[0] for f in fields], recs


def dbf_as_text_lines(path):
    """A dBase file as text lines, one per record, for comparison."""
    names, recs = read_dbf_records(path)
    lines = ["FIELDS " + " ".join(names)]
    for flag, vals in recs:
        lines.append(("*" if flag == "*" else "") +
                     " | ".join("%s=%s" % (n, v) for n, v in zip(names, vals)))
    return lines


def shx_record_count(path):
    size = os.path.getsize(path)
    return (size - 100) // 8
