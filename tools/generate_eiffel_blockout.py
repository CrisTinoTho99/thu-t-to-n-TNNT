#!/usr/bin/env python3
"""Generate the low-detail Eiffel Tower GLB used to smoke-test map imports."""

import json
import math
import struct
import sys
from pathlib import Path


class MeshBuilder:
    def __init__(self, color):
        self.color = color
        self.positions = []
        self.indices = []

    def add_beam(self, start, end, width):
        direction = normalize(subtract(end, start))
        reference = (0.0, 0.0, 1.0)
        if abs(direction[2]) > 0.9:
            reference = (0.0, 1.0, 0.0)
        axis_u = scale(normalize(cross(direction, reference)), width * 0.5)
        axis_v = scale(normalize(cross(direction, axis_u)), width * 0.5)

        corners = []
        for endpoint in (start, end):
            corners.extend(
                [
                    add(add(endpoint, scale(axis_u, -1)), scale(axis_v, -1)),
                    add(add(endpoint, axis_u), scale(axis_v, -1)),
                    add(add(endpoint, axis_u), axis_v),
                    add(add(endpoint, scale(axis_u, -1)), axis_v),
                ]
            )
        self.add_box_vertices(corners)

    def add_box(self, center, size):
        half = scale(size, 0.5)
        corners = [
            (center[0] - half[0], center[1] - half[1], center[2] - half[2]),
            (center[0] + half[0], center[1] - half[1], center[2] - half[2]),
            (center[0] + half[0], center[1] - half[1], center[2] + half[2]),
            (center[0] - half[0], center[1] - half[1], center[2] + half[2]),
            (center[0] - half[0], center[1] + half[1], center[2] - half[2]),
            (center[0] + half[0], center[1] + half[1], center[2] - half[2]),
            (center[0] + half[0], center[1] + half[1], center[2] + half[2]),
            (center[0] - half[0], center[1] + half[1], center[2] + half[2]),
        ]
        self.add_box_vertices(corners)

    def add_box_vertices(self, corners):
        base = len(self.positions)
        self.positions.extend(corners)
        for face in (
            (0, 1, 2, 3),
            (4, 7, 6, 5),
            (0, 4, 5, 1),
            (1, 5, 6, 2),
            (2, 6, 7, 3),
            (3, 7, 4, 0),
        ):
            a, b, c, d = (base + index for index in face)
            self.indices.extend((a, b, c, a, c, d))


def add(a, b):
    return tuple(x + y for x, y in zip(a, b))


def subtract(a, b):
    return tuple(x - y for x, y in zip(a, b))


def scale(vector, factor):
    return tuple(value * factor for value in vector)


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def cross(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def normalize(vector):
    length = math.sqrt(dot(vector, vector))
    if length == 0:
        raise ValueError("Cannot normalize a zero-length vector")
    return scale(vector, 1.0 / length)


def tower_width(height):
    profile = ((0, 62.5), (57, 43), (115, 28), (180, 18), (276, 8), (330, 1))
    for (low_y, low_width), (high_y, high_width) in zip(profile, profile[1:]):
        if height <= high_y:
            ratio = (height - low_y) / (high_y - low_y)
            return low_width + ratio * (high_width - low_width)
    return profile[-1][1]


def corners_at(height):
    half = tower_width(height) * 0.5
    return (
        (-half, height, -half),
        (half, height, -half),
        (half, height, half),
        (-half, height, half),
    )


def build_meshes():
    pillars = MeshBuilder((0.58, 0.34, 0.16, 1.0))
    levels = (0, 57, 115, 180, 276)
    for low_y, high_y in zip(levels, levels[1:]):
        for index in range(4):
            pillars.add_beam(
                corners_at(low_y)[index], corners_at(high_y)[index], 4.0
            )

    lattice = MeshBuilder((0.72, 0.48, 0.23, 1.0))
    brace_levels = (0, 57, 115, 180, 276, 300)
    for height in brace_levels:
        corners = corners_at(height)
        for index in range(4):
            lattice.add_beam(corners[index], corners[(index + 1) % 4], 1.5)
    for low_y, high_y in zip(brace_levels, brace_levels[1:]):
        low = corners_at(low_y)
        high = corners_at(high_y)
        for index in range(4):
            next_index = (index + 1) % 4
            lattice.add_beam(low[index], high[next_index], 1.2)
            lattice.add_beam(low[next_index], high[index], 1.2)

    platforms = MeshBuilder((0.46, 0.29, 0.14, 1.0))
    for height in (57, 115, 276):
        width = tower_width(height) * 1.12
        platforms.add_box((0.0, height, 0.0), (width, 2.5, width))

    antenna = MeshBuilder((0.78, 0.58, 0.30, 1.0))
    antenna.add_beam((0.0, 276.0, 0.0), (0.0, 330.0, 0.0), 2.4)
    antenna.add_box((0.0, 300.0, 0.0), (12.0, 2.0, 12.0))
    for height, width in ((312, 7), (322, 4)):
        antenna.add_beam(
            (0.0, height, -width * 0.5), (0.0, height, width * 0.5), 1.0
        )
        antenna.add_beam(
            (-width * 0.5, height, 0.0), (width * 0.5, height, 0.0), 1.0
        )

    return (
        ("E01_Pillars", pillars),
        ("E02_CrossBracing", lattice),
        ("E03_Platforms", platforms),
        ("E07_Antenna", antenna),
    )


def append_aligned(buffer, data):
    while len(buffer) % 4:
        buffer.append(0)
    offset = len(buffer)
    buffer.extend(data)
    return offset


def create_glb(output):
    binary = bytearray()
    buffer_views = []
    accessors = []
    meshes = []
    nodes = [{"name": "Eiffel_Blockout", "children": []}]

    for node_index, (name, builder) in enumerate(build_meshes(), start=1):
        position_bytes = b"".join(struct.pack("<3f", *position) for position in builder.positions)
        position_offset = append_aligned(binary, position_bytes)
        buffer_views.append(
            {
                "buffer": 0,
                "byteOffset": position_offset,
                "byteLength": len(position_bytes),
                "target": 34962,
            }
        )
        positions = builder.positions
        accessors.append(
            {
                "bufferView": len(buffer_views) - 1,
                "componentType": 5126,
                "count": len(positions),
                "type": "VEC3",
                "min": [min(p[axis] for p in positions) for axis in range(3)],
                "max": [max(p[axis] for p in positions) for axis in range(3)],
            }
        )
        position_accessor = len(accessors) - 1

        color_bytes = b"".join(
            struct.pack("<4f", *builder.color) for _ in builder.positions
        )
        color_offset = append_aligned(binary, color_bytes)
        buffer_views.append(
            {
                "buffer": 0,
                "byteOffset": color_offset,
                "byteLength": len(color_bytes),
                "target": 34962,
            }
        )
        accessors.append(
            {
                "bufferView": len(buffer_views) - 1,
                "componentType": 5126,
                "count": len(positions),
                "type": "VEC4",
            }
        )
        color_accessor = len(accessors) - 1

        index_bytes = struct.pack("<" + "I" * len(builder.indices), *builder.indices)
        index_offset = append_aligned(binary, index_bytes)
        buffer_views.append(
            {
                "buffer": 0,
                "byteOffset": index_offset,
                "byteLength": len(index_bytes),
                "target": 34963,
            }
        )
        accessors.append(
            {
                "bufferView": len(buffer_views) - 1,
                "componentType": 5125,
                "count": len(builder.indices),
                "type": "SCALAR",
                "min": [min(builder.indices)],
                "max": [max(builder.indices)],
            }
        )
        index_accessor = len(accessors) - 1

        meshes.append(
            {
                "name": name,
                "primitives": [
                    {
                        "attributes": {
                            "POSITION": position_accessor,
                            "COLOR_0": color_accessor,
                        },
                        "indices": index_accessor,
                        "mode": 4,
                    }
                ],
            }
        )
        nodes.append({"name": name, "mesh": node_index - 1})
        nodes[0]["children"].append(node_index)

    document = {
        "asset": {"version": "2.0", "generator": "VirtualLandmarkTour blockout"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": nodes,
        "meshes": meshes,
        "buffers": [{"byteLength": len(binary)}],
        "bufferViews": buffer_views,
        "accessors": accessors,
    }
    json_chunk = json.dumps(document, separators=(",", ":"), ensure_ascii=True).encode("utf-8")
    json_chunk += b" " * ((4 - len(json_chunk) % 4) % 4)
    binary.extend(b"\x00" * ((4 - len(binary) % 4) % 4))

    total_length = 12 + 8 + len(json_chunk) + 8 + len(binary)
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("wb") as stream:
        stream.write(struct.pack("<4sII", b"glTF", 2, total_length))
        stream.write(struct.pack("<I4s", len(json_chunk), b"JSON"))
        stream.write(json_chunk)
        stream.write(struct.pack("<I4s", len(binary), b"BIN\x00"))
        stream.write(binary)


if __name__ == "__main__":
    destination = (
        Path(sys.argv[1])
        if len(sys.argv) > 1
        else Path(__file__).resolve().parents[1]
        / "assets"
        / "maps"
        / "eiffel"
        / "models"
        / "eiffel_blockout.glb"
    )
    create_glb(destination)
    print("Generated", destination)
