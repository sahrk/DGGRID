#!/usr/bin/env python3
"""
cmpexamples.py: compare the DGGRID example outputs with a reference tree,
separating last-digit round-off from real change.

Every file is classed as
  IDENTICAL   byte-identical after the environment normalization below
  ROUNDOFF    only floats differ, each within tolerance
  STRUCTURAL  anything else: a missing or extra file, a changed line count,
              text or integer token, or a float beyond tolerance
and an example with no reference directory as NOREF (not a failure).

Rules (text files: txt kml geojson gen nbr chd ndx* meta.000N prj ...):
  - Lines are split on whitespace into tokens, and tokens into numbers and
    other text. Line counts, and the token sequence of each line, must match.
  - Non-number text must match exactly. Integers (cell ids, counts, ...)
    must match exactly.
  - Floats a, b printed with da, db decimals are equal when
    |a - b| <= 10^-max(da, db) (one unit in the last printed place),
    compared exactly in decimal; -0 equals 0. With --expect-change ABS_TOL
    the tolerance is max(10^-d, ABS_TOL).
  - Environment normalization, and nothing else: the dggrid log lines
      ** executing DGGRID version ... with GDAL version ... **
      type sizes: big int: ... / big double: ...
    and the last-update date in dBase (.dbf) headers (bytes 1-3).
  - Shapefiles: .shp files are compared through `ogrinfo -al -q` dumps
    (the DBF_DATE_LAST_UPDATE metadata line removed), .dbf files through
    their decoded records, .shx files by record count.

A STRUCTURAL file can be accepted only through --allow FILE. Each line of
that file is
    <example>/<file>[:<line>[,<line>...]]  <justification>
where <example>/<file> may be a glob pattern; the justification is
required. With line numbers the entry covers only STRUCTURAL differences on
those lines. Entries that no longer match anything are reported as STALE.

Output: per-file lines for everything not IDENTICAL (all files with -v),
per-example summaries, and a one-line total such as
    IDENTICAL 212 / ROUNDOFF 9 / STRUCTURAL 0 / NOREF 1 | max|d| 1e-07 at ...
The exit status is 1 if any STRUCTURAL difference is not allowed.

Usage (from the examples directory):
   python3 cmpexamples.py [--ref DIR] [--expect-change ABS_TOL]
                          [--allow FILE] [--out-root DIR] [--report FILE]
                          [-v] [examples...]
  --ref DIR       reference tree holding DIR/<ex>/ (default sampleOutput)
  --out-root DIR  compare DIR/<ex>/ instead of <ex>/outputfiles/
  --report FILE   also write the full report to FILE
"""

import argparse
import fnmatch
import os
import re
import sys
from decimal import Decimal

sys.dont_write_bytecode = True  # keep the examples directory clean
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import exampleslib as L  # noqa: E402

ENV_LINES = [
    (re.compile(rb"^\*\* executing DGGRID version .* \*\*$", re.M),
     b"** executing DGGRID version <env> **"),
    (re.compile(rb"^type sizes: .*$", re.M), b"type sizes: <env>"),
]
BINARY_EXT = {"shp", "shx", "dbf"}
IGNORE_NAMES = {".keep", ".DS_Store"}

# a number: not glued to a preceding letter/digit/_/. or a following
# letter/digit/_ (so "isea3h2.0001" or "1400000000000000a" stay text)
NUM_RE = re.compile(r"(?<![A-Za-z0-9_.])[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?(?![A-Za-z0-9_])")

MAX_DETAIL = 5


class FileResult:
    def __init__(self, relpath):
        self.path = relpath
        self.cls = "IDENTICAL"
        self.maxd = Decimal(0)
        self.maxd_loc = None
        self.details = []       # (line, reason, text)
        self.struct_lines = set()  # line numbers with STRUCTURAL diffs (0 = whole file)
        self.allowed_by = None

    def structural(self, line, reason, text):
        self.cls = "STRUCTURAL"
        self.struct_lines.add(line)
        if len(self.details) < MAX_DETAIL:
            self.details.append((line, reason, text))

    def roundoff(self):
        if self.cls == "IDENTICAL":
            self.cls = "ROUNDOFF"


def normalize_env(data):
    for rx, rep in ENV_LINES:
        data = rx.sub(rep, data)
    return data


def split_tokens(tok):
    """Split a whitespace token into [(is_number, text)]."""
    out = []
    pos = 0
    for m in NUM_RE.finditer(tok):
        if m.start() > pos:
            out.append((False, tok[pos:m.start()]))
        out.append((True, m.group(0)))
        pos = m.end()
    if pos < len(tok):
        out.append((False, tok[pos:]))
    return out


def is_int(s):
    return "." not in s and "e" not in s and "E" not in s


def decimals(s):
    """Decimal places of the last printed digit (can be negative with an
    exponent)."""
    mant, _, exp = s.lower().partition("e")
    d = len(mant.split(".", 1)[1]) if "." in mant else 0
    return d - (int(exp) if exp else 0)


def compare_lines(fr, a_lines, b_lines, abs_tol):
    if len(a_lines) != len(b_lines):
        fr.structural(0, "line count", "%d lines vs %d in reference"
                      % (len(a_lines), len(b_lines)))
    for i, (a, b) in enumerate(zip(a_lines, b_lines), 1):
        if a == b:
            continue
        ta, tb = a.split(), b.split()
        if ta == tb:
            fr.roundoff()  # whitespace only (as diff -wb): not a real change
            continue
        if len(ta) != len(tb):
            fr.structural(i, "token count", "%s | ref: %s" % (a.strip()[:150], b.strip()[:150]))
            continue
        line_struct = None
        for x, y in zip(ta, tb):
            if x == y:
                continue
            px, py = split_tokens(x), split_tokens(y)
            if len(px) != len(py):
                line_struct = "token '%s' vs '%s'" % (x, y)
                break
            for (nx, sx), (ny, sy) in zip(px, py):
                if sx == sy:
                    continue
                if not (nx and ny):
                    line_struct = "text '%s' vs '%s'" % (sx, sy)
                    break
                if is_int(sx) and is_int(sy):
                    if Decimal(sx) == 0 and Decimal(sy) == 0:
                        continue  # -0 vs 0
                    line_struct = "integer %s vs %s" % (sx, sy)
                    break
                d = max(decimals(sx), decimals(sy))
                tol = Decimal(10) ** (-d)
                if abs_tol is not None and abs_tol > tol:
                    tol = abs_tol
                delta = abs(Decimal(sx) - Decimal(sy))
                if delta > fr.maxd:
                    fr.maxd = delta
                    fr.maxd_loc = i
                if delta > tol:
                    line_struct = "float %s vs %s (|d| %s > tol %s)" % (
                        sx, sy, fmt_d(delta), fmt_d(tol))
                    break
            if line_struct:
                break
        if line_struct:
            fr.structural(i, line_struct, "%s | ref: %s" % (a.strip()[:150], b.strip()[:150]))
        else:
            fr.roundoff()


def fmt_d(d):
    return "%.3g" % float(d)


def read_text_lines(path):
    with open(path, "rb") as f:
        data = normalize_env(f.read())
    return data.decode("utf-8", errors="replace").splitlines()


def is_text_file(path):
    with open(path, "rb") as f:
        chunk = f.read(8192)
    if b"\0" in chunk:
        return False
    try:
        chunk.decode("utf-8")
        return True
    except UnicodeDecodeError:
        return False


def compare_file(rel, out, ref, abs_tol):
    fr = FileResult(rel)
    ext = rel.rsplit(".", 1)[-1].lower() if "." in os.path.basename(rel) else ""
    with open(out, "rb") as f:
        da = f.read()
    with open(ref, "rb") as f:
        db = f.read()
    if ext == "dbf" and len(da) > 4 and len(db) > 4:
        da = da[:1] + b"\0\0\0" + da[4:]
        db = db[:1] + b"\0\0\0" + db[4:]
    if ext not in BINARY_EXT:
        da, db = normalize_env(da), normalize_env(db)
    if da == db:
        return fr
    try:
        if ext == "shp":
            compare_lines(fr, L.ogrinfo_dump(out), L.ogrinfo_dump(ref), abs_tol)
            if fr.cls == "IDENTICAL":
                fr.roundoff()  # bytes differ but the dump does not
        elif ext == "dbf":
            compare_lines(fr, L.dbf_as_text_lines(out), L.dbf_as_text_lines(ref), abs_tol)
            if fr.cls == "IDENTICAL":
                fr.roundoff()
        elif ext == "shx":
            na, nb = L.shx_record_count(out), L.shx_record_count(ref)
            if na != nb:
                fr.structural(0, "record count", "%d records vs %d" % (na, nb))
            else:
                # the index only holds record offsets and lengths, which
                # follow the .shp; its content is covered by the .shp
                # comparison, so only the record count is compared
                fr.roundoff()
        elif is_text_file(out) and is_text_file(ref):
            compare_lines(fr, da.decode("utf-8", "replace").splitlines(),
                          db.decode("utf-8", "replace").splitlines(), abs_tol)
            if fr.cls == "IDENTICAL":
                fr.roundoff()  # e.g. only line endings differ
        else:
            fr.structural(0, "binary files differ", "")
    except Exception as e:
        fr.structural(0, "comparison failed", str(e))
    return fr


def list_files(d):
    res = set()
    for root, dirs, files in os.walk(d):
        dirs[:] = [x for x in dirs if not x.startswith(".")]
        for fn in files:
            if fn in IGNORE_NAMES:
                continue
            res.add(os.path.relpath(os.path.join(root, fn), d))
    return res


def sort_key(rel):
    # list the files of a shapefile set together: .shp, .dbf, .shx
    ext = rel.rsplit(".", 1)[-1].lower()
    return (rel.rsplit(".", 1)[0], {"shp": 0, "dbf": 1, "shx": 2}.get(ext, 0), rel)


def compare_example(ex, outdir, refdir, abs_tol):
    results = []
    if not os.path.isdir(outdir):
        fr = FileResult("(output directory)")
        fr.structural(0, "missing", outdir)
        return [fr]
    fo, fref = list_files(outdir), list_files(refdir)
    for rel in sorted(fref - fo):
        fr = FileResult(rel)
        fr.structural(0, "missing file", "present in reference only")
        results.append(fr)
    for rel in sorted(fo - fref):
        fr = FileResult(rel)
        fr.structural(0, "extra file", "not in reference")
        results.append(fr)
    for rel in sorted(fo & fref, key=sort_key):
        results.append(compare_file(rel, os.path.join(outdir, rel),
                                    os.path.join(refdir, rel), abs_tol))
    return results


def read_allow(path):
    entries = []
    with open(path) as f:
        for n, line in enumerate(f, 1):
            s = line.strip()
            if not s or s.startswith("#"):
                continue
            parts = s.split(None, 1)
            spec = parts[0]
            just = parts[1].strip() if len(parts) > 1 else ""
            if not just:
                sys.exit("%s:%d: allow entry '%s' has no justification" % (path, n, spec))
            lines = None
            m = re.match(r"^(.*):([\d,]+)$", spec)
            if m:
                spec = m.group(1)
                lines = set(int(x) for x in m.group(2).split(","))
            entries.append({"spec": spec, "lines": lines, "just": just,
                            "where": "%s:%d" % (path, n), "used": False})
    return entries


def apply_allow(ex, fr, entries):
    key = "%s/%s" % (ex, fr.path)
    cover = set()
    whole = None
    for e in entries:
        if not fnmatch.fnmatchcase(key, e["spec"]):
            continue
        if e["lines"] is None:
            whole = e
            break
        if fr.struct_lines & e["lines"]:
            cover |= e["lines"]
            e["used"] = True
    if whole is not None:
        whole["used"] = True
        fr.allowed_by = whole
        return True
    if cover and fr.struct_lines <= cover:
        fr.allowed_by = {"just": "line entries"}
        return True
    return False


def main():
    ap = argparse.ArgumentParser(
        description="Compare DGGRID example output with a reference tree, "
                    "separating round-off from real change.",
        epilog="See the module docstring (head of this file) for the rules.")
    ap.add_argument("examples", nargs="*", help="examples to compare (default: all in list)")
    ap.add_argument("--ref", default="sampleOutput", help="reference tree (default sampleOutput)")
    ap.add_argument("--expect-change", type=str, default=None, metavar="ABS_TOL",
                    help="float tolerance max(10^-d, ABS_TOL)")
    ap.add_argument("--allow", default=None, metavar="FILE",
                    help="accepted STRUCTURAL differences, with justifications")
    ap.add_argument("--list", default=os.path.join(L.EXAMPLES_DIR, "examples.lst"))
    ap.add_argument("--out-root", default=None,
                    help="compare DIR/<ex>/ instead of <ex>/outputfiles/")
    ap.add_argument("--report", default=None, help="also write the report to FILE")
    ap.add_argument("-v", "--verbose", action="store_true", help="list every file")
    args = ap.parse_args()

    abs_tol = Decimal(args.expect_change) if args.expect_change else None
    ref_root = args.ref if os.path.isabs(args.ref) else os.path.join(L.EXAMPLES_DIR, args.ref)
    out_root = os.path.abspath(args.out_root) if args.out_root else None
    entries = read_allow(args.allow) if args.allow else []
    exs = args.examples or L.read_example_list(args.list)

    lines_out = []

    def emit(s=""):
        print(s)
        lines_out.append(s)

    emit("cmpexamples: reference %s%s" % (
        ref_root, (", --expect-change %s" % args.expect_change) if abs_tol else ""))
    counts = {"IDENTICAL": 0, "ROUNDOFF": 0, "STRUCTURAL": 0, "ALLOWED": 0}
    noref = []
    gmax, gloc = Decimal(0), None
    unallowed = 0
    for ex in exs:
        refdir = os.path.join(ref_root, ex)
        outdir = os.path.join(out_root, ex) if out_root else \
            os.path.join(L.EXAMPLES_DIR, ex, "outputfiles")
        if not os.path.isdir(refdir):
            noref.append(ex)
            emit("NOREF      %s" % ex)
            continue
        results = compare_example(ex, outdir, refdir, abs_tol)
        ecount = {"IDENTICAL": 0, "ROUNDOFF": 0, "STRUCTURAL": 0, "ALLOWED": 0}
        emax, eloc = Decimal(0), None
        for fr in results:
            cls = fr.cls
            if cls == "STRUCTURAL" and apply_allow(ex, fr, entries):
                cls = "ALLOWED"
            ecount[cls] += 1
            counts[cls] += 1
            if cls == "STRUCTURAL":
                unallowed += 1
            if fr.maxd > emax:
                emax, eloc = fr.maxd, "%s/%s:%s" % (ex, fr.path, fr.maxd_loc)
            if cls != "IDENTICAL" or args.verbose:
                extra = ""
                if fr.maxd > 0:
                    extra = "  max|d| %s at line %s" % (fmt_d(fr.maxd), fr.maxd_loc)
                if cls == "ALLOWED":
                    extra += "  [allowed: %s]" % fr.allowed_by["just"]
                emit("  %-10s %s/%s%s" % (cls, ex, fr.path, extra))
                if cls in ("STRUCTURAL", "ALLOWED"):
                    for (ln, reason, text) in fr.details:
                        emit("      line %s: %s%s" % (ln, reason, ("\n        " + text) if text else ""))
                    if len(fr.struct_lines) > len(fr.details):
                        emit("      ... %d lines with STRUCTURAL differences in total"
                             % len([x for x in fr.struct_lines if x]))
        if emax > gmax:
            gmax, gloc = emax, eloc
        state = ("STRUCTURAL" if ecount["STRUCTURAL"] else
                 "ROUNDOFF" if ecount["ROUNDOFF"] or ecount["ALLOWED"] else "IDENTICAL")
        emit("%-10s %s: %s%s" % (state, ex, " / ".join(
            "%s %d" % (k, v) for k, v in ecount.items() if v),
            ("  max|d| %s" % fmt_d(emax)) if emax > 0 else ""))
    for e in entries:
        if not e["used"]:
            emit("STALE allow entry %s: %s" % (e["where"], e["spec"]))
    summary = "IDENTICAL %d / ROUNDOFF %d / STRUCTURAL %d%s / NOREF %d | max|d| %s%s" % (
        counts["IDENTICAL"], counts["ROUNDOFF"], counts["STRUCTURAL"],
        (" (+%d allowed)" % counts["ALLOWED"]) if counts["ALLOWED"] else "",
        len(noref), fmt_d(gmax) if gmax > 0 else "0",
        (" at %s" % gloc) if gloc else "")
    emit("")
    emit("SUMMARY: " + summary)
    if args.report:
        with open(args.report, "w") as f:
            f.write("\n".join(lines_out) + "\n")
    sys.exit(1 if unallowed else 0)


if __name__ == "__main__":
    main()
