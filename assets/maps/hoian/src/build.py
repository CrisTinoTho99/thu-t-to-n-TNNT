"""Procedural generator for Chua Cau (Lai Vien Kieu), Hoi An.
Fully parameterized architectural builder for Virtual Landmark Tour.
Usage:
    python src/build.py [--statue_layout west_monkey_east_dog]
                        [--deck_arch_cm 34.0]
                        [--old_tile_ratio 0.30]
                        [--weathering 0.40]
                        [--output models/chua_cau.glb]
"""
import argparse
from collections import defaultdict
import math
from pathlib import Path
import random
from details import add_shophouse, central_crest, deck_plank, landing, plaque_letters

from mesh import Mesh, tint, write_glb

# Palette calibrated to the August 2024 comprehensive restoration
C_WOOD_OLD = (88, 56, 38)
C_WOOD_NEW = (174, 122, 68)
C_WOOD_DARK = (58, 38, 28)
C_LIME_RED = (169, 58, 36)
C_WALL_WHITE = (205, 197, 178)
C_WALL_GRAY = (165, 155, 136)
C_TILE_NEW = (165, 105, 73)
C_TILE_OLD = (137, 99, 77)
C_TILE_MOSS = (126, 104, 79)
C_COBALT = (30, 74, 168)
C_PORCELAIN = (236, 239, 241)
C_STONE_DRY = (142, 135, 120)
C_STONE_WET = (63, 74, 51)
C_GOLD = (217, 168, 63)
C_SEAL_RED = (185, 35, 28)
C_EYE_RED = (179, 32, 26)


def build_scene(statue_layout="west_monkey_east_dog",
                deck_arch_cm=34.0,
                old_tile_ratio=0.30,
                weathering=0.40,
                seed=1719):
    rng = random.Random(seed)
    scene = defaultdict(Mesh)

    def varied(c, k=0.08):
        return tint(c, rng.uniform(1.0 - k, 1.0 + k))

    arch_m = deck_arch_cm / 100.0

    def floor_y(x):
        # Arched deck camber peaking at bridge center (x = 0)
        return 1.16 + arch_m * max(0.0, 1.0 - (x / 9.0) ** 2)

    def cornice(mesh, x, y, z, w, d, color):
        for h, off, extra in [(0.09, 0.0, 0.02), (0.07, 0.075, 0.10), (0.055, 0.135, 0.16)]:
            mesh.box((x, y + off, z), (w + extra, h, d + extra), color)

    # -------------------------------------------------------------------------
    # H01_Masonry: Stone piers, cutwaters, abutments & perforated wall
    # -------------------------------------------------------------------------
    piers = scene['H01_Masonry']
    pier_xs = [-5.7, -3.22, -1.52, 1.52, 3.22, 5.7]
    for px in pier_xs:
        for j in range(8):
            h = 0.25
            width = 0.49 + max(0, j - 4) * 0.095
            stone_c = C_STONE_DRY
            for k in range(5):
                piers.box((px, -0.6 + j * h, -1.68 + k * 0.84),
                          (width, h - 0.008, 0.835), varied(stone_c, 0.09))
            for sign in (-1, 1):
                z = sign * 2.08
                nose = z + sign*.20
                for flank in (-1, 1):
                    edge = px + flank*width/2
                    piers.face([(edge,-.72+j*h,z),(px,-.72+j*h,nose),
                                (px,-.47+j*h,nose),(edge,-.47+j*h,z)],varied(stone_c,.06))
                piers.face([(px-width/2,-.47+j*h,z),(px,-.47+j*h,nose),
                            (px+width/2,-.47+j*h,z)],stone_c)

    # Parapet walls with decorative breeze ventilation openings (ô thoáng bông gió)
    for i in range(95):
        x = -9.0 + (i + 0.5) * 18.0 / 95.0
        y = floor_y(x)
        for side in (-1, 1):
            wall_color = varied(C_WALL_GRAY if i % 4 == 0 else C_WALL_WHITE, 0.02)
            # Solid lower parapet
            piers.box((x, y - 0.25, side * 1.46), (18.0 / 95.0 + 0.002, 0.36, 0.35), wall_color)
            # Parapet coping / kerb
            piers.box((x, y + 0.04, side * 1.46), (18.0 / 95.0 + 0.002, 0.08, 0.38), varied(C_STONE_DRY, 0.04))

    # Stone approach steps at both portals
    for s in (-1, 1):
        for st in range(3):
            step_x = s * (9.15 + st * 0.32)
            step_y = 1.16 - st * 0.12
            piers.box((step_x, step_y, 0), (0.34, 0.12, 2.80), varied(C_STONE_DRY, 0.05))

    # -------------------------------------------------------------------------
    # H02_Deck: Arched timber deck planks & under-deck joists
    # -------------------------------------------------------------------------
    deck = scene['H02_Deck']
    num_planks = 95
    for i in range(num_planks):
        x = -9.0 + (i + 0.5) * 18.0 / num_planks
        y = floor_y(x)
        # Ratio of fresh planks vs preserved historic planks
        is_fresh = rng.random() > (1.0 - weathering * 0.5)
        c_plank = varied(C_WOOD_NEW if is_fresh else C_WOOD_OLD, 0.06)
        # Main central cambered walkway (width 3.18m)
        deck_plank(deck, x, y, 18.0 / num_planks - .004, c_plank)
        # Longitudinal sleeper beams underneath visible from water level
        if i % 3 == 0:
            deck.box((x, y - 0.16, -1.0), (18.0 / num_planks * 3.1, 0.14, 0.18), C_WOOD_DARK)
            deck.box((x, y - 0.16, 1.0), (18.0 / num_planks * 3.1, 0.14, 0.18), C_WOOD_DARK)
            deck.box((x, y - 0.18, 0.0), (18.0 / num_planks * 3.1, 0.16, 0.22), C_WOOD_DARK)

    # -------------------------------------------------------------------------
    # H03_Timber: 7-bay timber frame, 8 bents, chamfered posts, rafters & trusses
    # -------------------------------------------------------------------------
    timber = scene['H03_Timber']
    xs = [-9.0 + i * 18.0 / 7.0 for i in range(8)]
    for i, x in enumerate(xs):
        y = floor_y(x)
        # Posts on both sides of walkway
        for sign in (-1, 1):
            z = sign * 1.42
            # Major columns: 60% preserved historic timber, 40% reinforced/new
            col_fresh = (i in (1, 6) and sign < 0) or (i == 4 and sign > 0)
            col_color = C_WOOD_NEW if col_fresh else C_WOOD_OLD
            # Stone plinth base (chân tảng đá)
            timber.box((x, y + 0.068, z), (0.32, 0.14, 0.32), C_STONE_DRY)
            # Main square post with chamfered corners (cột vuông vuốt tròn 4 cạnh)
            timber.rod((x, y + 0.12, z), (x, y + 2.49, z), 0.114, col_color, 12, end=0.100)
            # Bracket sets (chồng đấu con sơn)
            for j, (w, h) in enumerate([(0.29, 0.10), (0.44, 0.09), (0.59, 0.085)]):
                timber.box((x, y + 2.17 + j * 0.08, z), (w, h, 0.26), varied(C_WOOD_OLD, 0.05))
            # Diagonal braces (kèo phụ)
            for s in (-1, 1):
                timber.rod((x, y + 1.94, z), (x + s * 0.46, y + 2.42, z), 0.063, C_WOOD_OLD, 4)

        # Transverse tie beam spanning across the bridge (xà ngang)
        beam_fresh = (i % 3 == 1)
        timber.box((x, y + 2.48, 0), (0.17, 0.17, 3.48), C_WOOD_NEW if beam_fresh else C_WOOD_OLD)
        # Exposed shoulders, through-tenons and drawbore pegs in the approach bay.
        if i in (5, 6):
            for s in (-1, 1):
                timber.box((x,y+2.48,s*1.78),(.10,.085,.12),tint(C_WOOD_OLD,1.15))
                timber.box((x,y+2.23,s*1.42),(.28,.11,.33),C_WOOD_OLD)
                timber.rod((x-.078,y+2.23,s*1.42),(x+.15,y+2.23,s*1.42),
                           .024,C_WOOD_NEW,12)
        # King post / roof truss rafters (trụ đội, kẽ nóc)
        for sign in (-1, 1):
            timber.rod((x, y + 2.49, sign * 1.55), (x, y + 3.10, 0), 0.092, C_WOOD_OLD, 4)

        # Longitudinal purlins connecting bents
        if i < 7:
            nx = xs[i + 1]
            ny = floor_y(nx)
            for s in (-1, 1):
                z = 1.44 * s
                timber.rod((x, y + 2.38, z), (nx, ny + 2.38, z), 0.112, C_WOOD_OLD, 4)
                # Upper purlin
                timber.rod((x, y + 2.85, z * 0.5), (nx, ny + 2.85, z * 0.5), 0.088, C_WOOD_OLD, 4)
            # Ridge purlin along bridge axis (xà nóc chạm niên đại trùng tu)
            timber.rod((x, y + 3.12, 0), (nx, ny + 3.12, 0), 0.095, C_WOOD_OLD, 6)

    # -------------------------------------------------------------------------
    # H04_Rails: Handrails & turned balusters (tay vịn bo tròn & con tiện)
    # -------------------------------------------------------------------------
    rail = scene['H04_Rails']
    for i in range(7):
        x = xs[i]
        y = floor_y(x)
        nx = xs[i + 1]
        ny = floor_y(nx)
        for s in (-1, 1):
            z = 1.44 * s
            # Skip balustrade where shrine entrance connects (north side, mid-span)
            if s < 0 and -2.2 < x < 1.8:
                continue
            # Smooth rounded handrail at ~0.85m height
            for h, rad in [(0.17, 0.065), (0.68, 0.070), (0.85, 0.072)]:
                rail.rod((x, y + h, z), (nx, ny + h, z), rad, varied(C_WOOD_OLD, 0.04), 10)
            # Vertical balusters (con tiện gỗ)
            for j in range(1, 13):
                xx = x + (nx - x) * j / 13.0
                yy = floor_y(xx)
                rail.rod((xx, yy + 0.17, z), (xx, yy + 0.68, z), 0.034, varied(C_WOOD_DARK, 0.08), 6)

    # -------------------------------------------------------------------------
    # H05_Portals: Portal pavilions, Buddha's hand & pomegranate reliefs,
    #              commemorative stelae, and guardian animal statues (Thần Hầu & Thần Cẩu)
    # -------------------------------------------------------------------------
    gate = scene['H05_Portals']
    for s in (-1, 1):
        x = s * 7.72
        for side in (-1, 1):
            z = side * 1.50
            # Red lime wash wall (vách hồi quét vôi đỏ son)
            gate.box((x, 2.29, z), (2.64, 2.42, 0.32), varied(C_LIME_RED, 0.03))
            cornice(gate, x, 1.19, z, 2.64, 0.34, tint(C_LIME_RED, 0.86))
            cornice(gate, x, 3.40, z, 2.64, 0.34, tint(C_LIME_RED, 1.10))
            gate.box((s * 8.92, 2.28, side * 1.13), (0.39, 2.30, 0.92), C_LIME_RED)
            cornice(gate, s * 8.92, 1.24, side * 1.13, 0.39, 0.92, tint(C_LIME_RED, 0.86))

            # Commemorative stone stelae embedded in portal wall (4 bia đá lớn 1817-1917)
            stele_x = s * 7.95
            gate.box((stele_x, 2.15, z - side * 0.12), (0.85, 1.18, 0.06), varied(C_STONE_DRY, 0.06))
            gate.box((stele_x, 2.76, z - side * 0.12), (0.90, 0.14, 0.08), tint(C_STONE_DRY, 1.08))

            # Architectural reliefs on portal facades:
            # East end (s > 0, Trần Phú): Buddha's Hand fruit (Phù điêu Phật Thủ)
            # West end (s < 0, Minh Khai): Pomegranate fruit pair (Phù điêu Hai Quả Lựu)
            if s > 0:
                # Buddha's hand fruit relief (ngón tay Phật xòe uyển chuyển)
                fx = s * 9.16
                fz = side * 1.13
                gate.rod((fx, 2.18, fz), (fx, 2.42, fz), 0.065, C_GOLD, 8, end=0.040)
                for f_ang in (-0.35, -0.15, 0.0, 0.15, 0.35):
                    gate.rod((fx, 2.38, fz), (fx, 2.56 + math.cos(f_ang) * 0.08, fz + f_ang * 0.22),
                             0.022, C_GOLD, 6, end=0.010)
            else:
                # Pomegranate relief (quả lựu tròn căng hai bên)
                fx = s * 9.16
                fz = side * 1.13
                for p_off in (-0.09, 0.09):
                    gate.rod((fx, 2.32, fz + p_off), (fx, 2.46, fz + p_off), 0.075, C_LIME_RED, 8, end=0.055)
                    gate.rod((fx, 2.46, fz + p_off), (fx, 2.53, fz + p_off), 0.025, C_GOLD, 6, end=0.040)

        # Portal arch beam
        gate.box((s * 8.92, 3.48, 0), (0.46, 0.18, 3.26), tint(C_LIME_RED, 0.91))

        # ---------------------------------------------------------------------
        # Guardian Animal Statues (Linh thú canh giữ hai đầu cầu)
        # Default layout:
        #   - East portal (s > 0, Trần Phú): Dog statues (Thần Cẩu)
        #   - West portal (s < 0, Minh Khai): Monkey statues (Thần Hầu) holding peach
        # ---------------------------------------------------------------------
        sx = s * 7.44
        sy = floor_y(sx)
        for side in (-1, 1):
            sz = side * 1.14
            # Stone plinth base (bệ đá chạm)
            gate.box((sx, sy + 0.12, sz), (0.34, 0.24, 0.34), C_STONE_DRY)
            gate.box((sx, sy + 0.25, sz), (0.37, 0.04, 0.37), tint(C_STONE_DRY, 1.08))
            # Bronze/ceramic incense altar bowl in front of statue
            inward = -s
            ibx = sx + inward * 0.16
            gate.rod((ibx, sy + 0.27, sz), (ibx, sy + 0.35, sz), 0.045, (140, 125, 95), 8, end=0.055)

            is_monkey = (s < 0) if statue_layout == "west_monkey_east_dog" else (s > 0)
            if is_monkey:
                # Monkey Guardian (Thần Hầu) seated cross-legged, holding peach (ôm quả đào)
                gate.box((sx, sy + 0.34, sz), (0.24, 0.14, 0.26), C_WOOD_DARK)
                # Torso
                gate.rod((sx, sy + 0.34, sz), (sx, sy + 0.60, sz), 0.105, (85, 55, 40), 8, end=0.088)
                # Head
                gate.rod((sx, sy + 0.60, sz), (sx, sy + 0.74, sz), 0.082, (95, 60, 42), 8, end=0.062)
                # Muzzle facing bridge center
                gate.box((sx + inward * 0.08, sy + 0.66, sz), (0.09, 0.06, 0.08), tint((95, 60, 42), 0.9))
                # Ears
                gate.rod((sx, sy + 0.70, sz - 0.07), (sx, sy + 0.70, sz + 0.07), 0.026, C_WOOD_DARK, 6)
                # Arms holding peach before chest (ôm quả đào trước ngực)
                gate.rod((sx + 0.03, sy + 0.50, sz - 0.08), (sx + inward * 0.08, sy + 0.44, sz), 0.033, C_WOOD_DARK, 6)
                gate.rod((sx + 0.03, sy + 0.50, sz + 0.08), (sx + inward * 0.08, sy + 0.44, sz), 0.033, C_WOOD_DARK, 6)
                # Peach fruit (quả đào thếp vàng / hồng gốm)
                gate.rod((sx + inward * 0.10, sy + 0.41, sz), (sx + inward * 0.10, sy + 0.48, sz), 0.038, (198, 142, 68), 6, end=0.012)
            else:
                # Dog Guardian (Thần Cẩu) seated erect on alert guard
                gate.box((sx, sy + 0.34, sz), (0.26, 0.15, 0.26), C_WOOD_DARK)
                # Upright chest
                gate.rod((sx - inward * 0.02, sy + 0.35, sz), (sx + inward * 0.04, sy + 0.62, sz), 0.108, (85, 55, 40), 8, end=0.086)
                # Forelegs standing straight
                for sign in (-1, 1):
                    gate.rod((sx + inward * 0.08, sy + 0.27, sz + sign * 0.06),
                             (sx + inward * 0.06, sy + 0.50, sz + sign * 0.06), 0.032, C_WOOD_DARK, 6)
                # Head & alert muzzle
                gate.rod((sx + inward * 0.04, sy + 0.62, sz), (sx + inward * 0.08, sy + 0.75, sz), 0.078, (95, 60, 42), 8, end=0.058)
                gate.box((sx + inward * 0.14, sy + 0.68, sz), (0.09, 0.06, 0.07), tint((95, 60, 42), 0.88))
                # Alert pointed ears
                for sign in (-1, 1):
                    gate.rod((sx + inward * 0.03, sy + 0.74, sz + sign * 0.05),
                             (sx + inward * 0.02, sy + 0.82, sz + sign * 0.06), 0.022, C_WOOD_DARK, 4, end=0.008)
                # Curled tail along flank
                gate.rod((sx - inward * 0.10, sy + 0.30, sz),
                         (sx - inward * 0.08, sy + 0.48, sz + 0.09), 0.025, C_WOOD_DARK, 5, end=0.012)

    # -------------------------------------------------------------------------
    # H06_RoofTiles & H07_Ceramics: True 3D yin-yang tiles and porcelain ridge crests
    # -------------------------------------------------------------------------
    tiles = scene['H06_RoofTiles']
    glaze = scene['H07_Ceramics']
    frame = scene['H03_Timber']

    def build_roof(cx, cz, length, span, crest, rot=False, central=False):
        def point(a, b, y):
            return (cx + b, y, cz + a) if rot else (cx + a, y, cz + b)

        half = length / 2.0

        def height(a, b):
            t = abs(b) / span
            return crest - 1.14 * (1.30 * t - 0.30 * t * t) + 0.115 * (abs(a) / half) ** 6

        for s in (-1, 1):
            # Timber under-eave ceiling board
            for i in range(6):
                a = -half + length * i / 6.0
                aa = a + length / 6.0
                frame.face([point(a, 0, height(a, 0) - 0.09),
                            point(aa, 0, height(aa, 0) - 0.09),
                            point(aa, s * span, height(aa, span) - 0.09),
                            point(a, s * span, height(a, span) - 0.09)], C_WOOD_DARK)

            # True interlocking concave and convex tile courses
            rows = max(4, round(length / 0.166))
            pitch = length / rows
            courses = 9
            for i in range(rows):
                a = -half + (i + 0.5) * pitch
                for j in range(courses):
                    b0 = span * j / courses
                    b1 = span * (j + 1) / courses + 0.016
                    # Mix of 30% old weathered tiles and 70% fresh terracotta
                    is_old = rng.random() < old_tile_ratio
                    tile_base = (C_TILE_MOSS if rng.random() < 0.25 else C_TILE_OLD) if is_old else C_TILE_NEW
                    tile_col = varied(tile_base, 0.045)

                    for cap in (False, True):
                        mid = a + (pitch * 0.5 if cap else 0)
                        radius = pitch * (0.23 if cap else 0.52)
                        lift = 0.035 if cap else 0.02
                        stations = []
                        for b, step in ((b0, 0.012), (b1, 0)):
                            ring = []
                            for k in range(5):
                                ang = math.pi * k / 4.0
                                ax = mid - radius * math.cos(ang)
                                rise = radius * math.sin(ang) * (1.0 if cap else -0.41)
                                ring.append(point(ax, s * b, height(ax, b) + lift + rise + step))
                            stations.append(ring)
                        for k in range(4):
                            pts = [stations[0][k], stations[0][k + 1], stations[1][k + 1], stations[1][k]]
                            if (s == 1 and not rot) or (s == -1 and rot):
                                pts.reverse()
                            tiles.face(pts, tint(tile_col, 1.09) if cap else tile_col)
                        # Tile drip lip at eave ends
                        lip = stations[1]
                        for k in range(4):
                            p, q = lip[k:k + 2]
                            tiles.face([p, q, (q[0], q[1] - 0.023, q[2]), (p[0], p[1] - 0.023, p[2])],
                                       tint(tile_col, 0.82))

                # Exposed wooden rafters under eave (rui gỗ đỡ mái)
                if i % 2 == 0:
                    pos = point(a, s * span, height(a, span) - 0.018)
                    frame.rod(point(a, 0, height(a, 0) - 0.15), pos, 0.047, varied(C_WOOD_OLD), 8)

            # Continuous fascia beam beneath tile lips
            for i in range(12):
                a = -half + length * i / 12.0
                aa = a + length / 12.0
                frame.rod(point(a, s * span, height(a, span) - 0.096),
                          point(aa, s * span, height(aa, span) - 0.096), 0.068, C_WOOD_OLD, 4)

        # Ridge beam (bờ nóc)
        def crown(a):
            return height(a, 0) + 0.11 + 0.22 * (abs(a) / half) ** 8

        for i in range(30):
            a = -half + length * i / 30.0
            aa = a + length / 30.0
            for h, r, c in [(0.0, 0.115, C_PORCELAIN), (0.095, 0.055, C_COBALT), (-0.075, 0.04, C_LIME_RED)]:
                glaze.rod(point(a, 0, crown(a) + h), point(aa, 0, crown(aa) + h), r, c, 8)

        if central:
            central_crest(glaze, point, half, crown)
            return

        # Restored porcelain dragons and clouds (lưỡng long tranh châu & mây lửa men lam-trắng)
        for anchor in [-half + 0.25, 0.0, half - 0.25]:
            y = crown(anchor) + 0.13
            for side in (-1, 1):
                for scale in (1.0, 0.66):
                    pts = []
                    for i in range(16):
                        t = i / 15.0
                        a = anchor + side * scale * (0.03 + 0.51 * t)
                        h = y + scale * (0.06 + 0.26 * math.sin(t * math.pi * 1.35) + 0.19 * t)
                        pts.append(point(a, 0, h))
                    glaze.path(pts, 0.045 * scale, C_PORCELAIN)
                    glaze.path([(px, py + 0.026, pz) for px, py, pz in pts], 0.027 * scale, C_COBALT)
                for j in range(3):
                    a = anchor + side * (0.07 + j * 0.13)
                    glaze.rod(point(a, 0, y + 0.10), point(a + side * 0.095, 0, y + 0.32 - j * 0.055),
                              0.046, C_COBALT, 6, end=0.009)
            # Inlaid antique porcelain roundels (đĩa cổ khảm men lam)
            for r, c in ((0.115, C_PORCELAIN), (0.080, C_COBALT), (0.043, C_PORCELAIN)):
                depth = 0.16 - r
                glaze.rod(point(anchor, depth, y + 0.20), point(anchor, depth + 0.03, y + 0.20), r, c, 16)

    # Bridge roofs: center span, intermediate spans, portal transverse roofs
    build_roof(0, 0, 4.6, 2.16, 5.24, central=True)
    for side in (-1, 1):
        build_roof(side * 4.43, 0, 4.26, 2.11, 5.05)
        build_roof(side * 7.89, 0, 4.65, 1.53, 4.84, True)
    # Shrine roof projecting north
    build_roof(0, -3.91, 5.48, 2.38, 5.30, True)

    # -------------------------------------------------------------------------
    # H08_Shrine: Bac De Tran Vu Shrine, screen & authentic shallow interior
    # -------------------------------------------------------------------------
    wall = scene['H08_Shrine']
    # Shrine foundation & walls
    wall.box((0, 1.34, -3.65), (4.25, 0.34, 4.75), C_WOOD_OLD)
    wall.box((0, 2.70, -5.93), (4.22, 2.62, 0.16), C_LIME_RED)
    for s in (-1, 1):
        wall.box((s * 2.05, 2.70, -3.80), (0.14, 2.62, 4.44), C_LIME_RED)
        # Wooden lattice screen: "thượng song hạ bản" (slats on top, solid panel below)
        wall.box((s * 1.44, 2.14, -1.56), (1.18, 1.29, 0.10), C_WOOD_OLD)
        for i in range(8):
            wall.box((s * (0.87 + i * 0.158), 3.12, -1.55), (0.038, 0.71, 0.065), varied(C_WOOD_OLD))
        for y in (1.58, 2.70, 3.49):
            wall.box((s * 1.48, y, -1.54), (1.28, 0.09, 0.15), C_WOOD_DARK)
        wall.box((s * 0.80, 2.62, -1.53), (0.14, 2.20, 0.17), C_WOOD_OLD)
    wall.box((0, 3.51, -1.54), (4.28, 0.13, 0.22), C_WOOD_OLD)
    # The two door eyes project from a solid lintel in front of the recessed screen.
    wall.box((0,3.29,-1.41),(1.78,.27,.27),C_WOOD_OLD)
    wall.box((0,3.42,-1.40),(1.88,.046,.31),tint(C_WOOD_OLD,1.08))
    for s in (-1,1):
        wall.box((s*.48,3.64,-1.42),(.085,.42,.19),C_WOOD_DARK)

    # Shallow sacred interior visible through the screen
    # Red altar table
    wall.box((0, 1.95, -4.50), (1.80, 0.80, 0.85), (145, 38, 30))
    wall.box((0, 2.37, -4.50), (1.92, 0.08, 0.95), C_WOOD_DARK)
    for panel_x in (-.55,0,.55):
        wall.box((panel_x,1.98,-4.065),(.48,.49,.034),(110,48,33))
        wall.box((panel_x,1.98,-4.043),(.42,.42,.018),(150,66,41))
    wall.box((0,2.28,-4.052),(1.85,.047,.08),tint(C_GOLD,.77))
    # Bronze incense burner & candlestick stands
    wall.rod((0, 2.42, -4.35), (0, 2.62, -4.35), 0.085, (155, 138, 92), 10, end=0.095)
    for cs in (-0.55, 0.55):
        wall.rod((cs, 2.42, -4.40), (cs, 2.70, -4.40), 0.030, (155, 138, 92), 6, end=0.015)
        # Candle flame point
        wall.rod((cs, 2.70, -4.40), (cs, 2.78, -4.40), 0.018, (245, 185, 75), 4, end=0.005)
    # Wooden statue of Bac De Tran Vu (Huyen Thien Dai De) ~0.5m tall
    # Red face, dark ceremonial robe, holding snake/sword, stepping on tortoise
    statue_y = 2.42
    wall.box((0, statue_y + 0.06, -4.68), (0.42, 0.12, 0.35), C_STONE_DRY)
    # Black tortoise at feet
    wall.box((0, statue_y + 0.14, -4.68), (0.28, 0.08, 0.28), (42, 42, 40))
    # Robed body
    wall.rod((0, statue_y + 0.18, -4.68), (0, statue_y + 0.45, -4.68), 0.12, (52, 48, 62), 8, end=0.095)
    # Red face
    wall.rod((0, statue_y + 0.45, -4.68), (0, statue_y + 0.58, -4.68), 0.065, (175, 52, 42), 8, end=0.045)
    # Golden crown & sword
    wall.box((0, statue_y + 0.59, -4.68), (0.09, 0.06, 0.09), C_GOLD)
    wall.rod((0.14, statue_y + 0.28, -4.62), (0.14, statue_y + 0.52, -4.62), 0.014, (180, 180, 185), 4)

    # -------------------------------------------------------------------------
    # H09_Plaque: Plaque (145 x 59 cm) "Lai Vien Kieu" (來遠橋) & Chau An Seal
    # -------------------------------------------------------------------------
    board = scene['H09_Plaque']
    cy = 3.78
    z = -1.30
    # Plaque panel 145 x 59 cm with frame
    board.box((0, cy, z), (1.45, 0.59, 0.075), C_SEAL_RED)
    for side in (-1, 1):
        board.box((0, cy + side * 0.265, z + 0.043), (1.45, 0.055, 0.045), C_GOLD)
        board.box((side * 0.697, cy, z + 0.043), (0.055, 0.51, 0.045), C_GOLD)
        # Golden floral bead relief along frame
        for j in range(38):
            x = -0.66 + j * 1.32 / 37.0
            board.rod((x, cy + side * 0.265 + 0.014 * math.sin(j * 0.7), z + 0.069),
                      (x + 0.024, cy + side * 0.265 + 0.014 * math.sin((j + 1) * 0.7), z + 0.069),
                      0.007, C_GOLD, 5)

    plaque_letters(board, cy, z, C_GOLD)

    # Lord Nguyen Phuc Chu's Chau An Imperial Seal on the left of plaque
    # Square red seal mark with gold characters
    for seal_y in (cy-.07,cy-.155):
        board.box((-.55,seal_y,z+.042),(.084,.080,.009),tint(C_GOLD,.83))
        board.box((-.55,seal_y,z+.047),(.066,.061,.005),(143,85,48))
    board.rod((.572,cy+.07,z+.042),(.572,cy+.07,z+.048),.029,tint(C_GOLD,.82),20)

    # -------------------------------------------------------------------------
    # H10_DoorEyes: Authentic 4-chrysanthemum Yin-Yang Door Eyes (Đôi mắt cửa)
    # -------------------------------------------------------------------------
    eyes = scene['H10_DoorEyes']
    for s in (-1, 1):
        x = s * 0.56
        yy = 3.28
        # Concentric turned wooden discs
        for r, d, c in ((0.126, 0.045, C_WOOD_OLD),
                        (0.110, 0.073, C_EYE_RED),
                        (0.082, 0.092, C_GOLD),
                        (0.064, 0.103, C_WOOD_DARK),
                        (0.034, 0.115, C_GOLD)):
            eyes.rod((x, yy, z + d - 0.023), (x, yy, z + d), r, c, 20)
        # 4 carved chrysanthemum petals (4 bông cúc quanh xoáy lưỡng nghi)
        for j in range(4):
            a = j * math.pi / 2.0 + math.pi / 4.0
            eyes.rod((x + 0.088 * math.cos(a), yy + 0.088 * math.sin(a), z + 0.068),
                     (x + 0.088 * math.cos(a), yy + 0.088 * math.sin(a), z + 0.088),
                     0.014, C_GOLD, 8)

    # -------------------------------------------------------------------------
    # ENV: Quays, Old Town, Water & Foliage
    # -------------------------------------------------------------------------
    bank = scene['ENV_Quays']
    town = scene['ENV_OldTown']
    water = scene['ENV_Water']

    # Water surface spanning the canal
    water.face([(-32, -0.23, -48), (-32, -0.23, 48), (32, -0.23, 48), (32, -0.23, -48)], (62, 108, 98))

    # Canal quays and stone embankments
    for s in (-1, 1):
        bank.box((s * 18, 0.37, 0), (23.8, 1.56, 84), (154, 146, 124))
        for j in range(4):
            for k in range(70):
                zz = -42 + k * 1.22 + (j % 2) * 0.60
                bank.box((s * 6.08, -0.2 + j * 0.37, zz), (0.16, 0.362, 1.215), varied(C_STONE_DRY))
        for k in range(107):
            zz = -40 + k * 0.75
            bank.box((s * 6.60, 1.15, zz), (1.17, 0.092, 0.735), varied((181, 171, 150)))

    # Foreground south bank landing for the 1.6m eye-level view
    landing(bank)
    bank.box((0, 1.01, 19.1), (15.4, 0.31, 5.6), (164, 157, 136))
    for k in range(22):
        bank.box((-7.55 + 0.72 * k, 1.165, 16.42), (0.707, 0.09, 0.30), varied(C_WALL_WHITE))

    # Old Town shophouses (nhà phố cổ tường vàng Hội An)
    for s in (-1, 1):
        for j in range(1,4):
            x = s * (12.4 + (j % 2) * 3.0)
            z = -5 - j * 5.8
            w = 4.6
            d = 4.2
            h = rng.uniform(3.4, 5.1)
            c = varied((187, 151, 92), 0.15)
            town.box((x, 1.16 + h / 2.0, z), (w, h, d), c)
            for side in (-1, 1):
                town.face([(x - w * 0.58, 1.16 + h, z + side * d * 0.57),
                           (x + w * 0.58, 1.16 + h, z + side * d * 0.57),
                           (x + w * 0.58, 2.32 + h, z),
                           (x - w * 0.58, 2.32 + h, z)],
                          varied((124, 94, 78)))
            for xx in (-1.2, 0, 1.2):
                town.box((x + xx, 2.39, z + d / 2.0 + 0.02), (0.73, 2.34, 0.07), varied((72, 76, 61)))
                for b in range(8):
                    town.box((x + xx, 2.08 + 0.125 * b, z + d / 2.0 + 0.067), (0.62, 0.039, 0.04), C_WOOD_DARK)

    # A short street recedes through each portal. The offset end facade closes
    # the line of sight while leaving the pedestrian lane between the two rows.
    facade_colors = [(198,163,103),(183,148,97),(201,181,135),(169,147,106)]
    for end in (-1,1):
        for side in (-1,1):
            for i, distance in enumerate((12.4,17.35,22.3)):
                frontage = 4.6 if i != 1 else 4.95
                add_shophouse(town,end*distance,side*(3.35+.24*((i+end)%3)), -side,
                              frontage,3.0+.28*((i+side)%3),facade_colors[(i+side+end)%4])
        add_shophouse(town,end*27.0,end*.62,-end,6.1,3.9,facade_colors[1+end],turn=True)

    # Distant foliage
    trees = scene['ENV_Foliage']
    for i in range(28):
        x = rng.choice([-1, 1]) * rng.uniform(8.7, 25)
        z = rng.uniform(-32, -7)
        h = rng.uniform(3.5, 7.5)
        trees.rod((x, 1, z), (x, h, z), 0.13, C_WOOD_DARK, 7, end=0.065)
        for k in range(160):
            a = rng.random() * math.tau
            elev = rng.uniform(-1, 1)
            radius = rng.random() ** (1 / 3) * 2.1
            dx = radius * math.sqrt(1 - elev * elev) * math.cos(a)
            dz = radius * math.sqrt(1 - elev * elev) * math.sin(a)
            xx = x + dx
            zz = z + dz
            yy = h + elev * radius * 0.79
            d = rng.uniform(0.12, 0.43)
            shade = varied((76 + int(13 * elev), 104 + int(16 * elev), 68), 0.18)
            trees.face([(xx - d, yy, zz), (xx, yy + 0.18, zz - d),
                        (xx + d, yy + 0.04, zz), (xx, yy - 0.10, zz + d)], shade)

    return scene


def main():
    parser = argparse.ArgumentParser(description="Chua Cau 3D Model Generator")
    parser.add_argument("--statue_layout", choices=["west_monkey_east_dog", "west_dog_east_monkey"],
                        default="west_monkey_east_dog",
                        help="Placement of guardian animals (default: West=Monkey, East=Dog)")
    parser.add_argument("--deck_arch_cm", type=float, default=34.0,
                        help="Deck camber arch height in cm (default: 34.0)")
    parser.add_argument("--old_tile_ratio", type=float, default=0.30,
                        help="Ratio of preserved historic tiles (default: 0.30)")
    parser.add_argument("--weathering", type=float, default=0.40,
                        help="Weathering factor for wood & stone (default: 0.40)")
    parser.add_argument("--output", type=str, default="",
                        help="Output path for GLB file")
    args = parser.parse_args()

    out_path = Path(args.output) if args.output else Path(__file__).resolve().parents[1] / "models/chua_cau.glb"

    print(f"Building Chua Cau 3D Model with parameters:")
    print(f"  statue_layout:  {args.statue_layout}")
    print(f"  deck_arch_cm:   {args.deck_arch_cm} cm")
    print(f"  old_tile_ratio: {args.old_tile_ratio}")
    print(f"  weathering:     {args.weathering}")
    print(f"  output:         {out_path}")

    scene = build_scene(statue_layout=args.statue_layout,
                        deck_arch_cm=args.deck_arch_cm,
                        old_tile_ratio=args.old_tile_ratio,
                        weathering=args.weathering)

    write_glb(scene, out_path)
    monument_nodes = [k for k in scene if k.startswith('H')]
    env_nodes = [k for k in scene if k.startswith('ENV')]
    tri_monument = sum(len(scene[k].idx) // 3 for k in monument_nodes)
    tri_env = sum(len(scene[k].idx) // 3 for k in env_nodes)
    total_tri = tri_monument + tri_env

    print(f"Build complete:")
    print(f"  Monument parts: {len(monument_nodes)} nodes, {tri_monument:,} triangles (Budget <= 400,000)")
    print(f"  Environment:    {len(env_nodes)} nodes, {tri_env:,} triangles (Budget <= 100,000)")
    print(f"  Total:          {len(scene)} nodes, {total_tri:,} triangles")


if __name__ == '__main__':
    main()

