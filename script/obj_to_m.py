#!/usr/bin/env python
# obj_to_m.py
#
# Converts a .obj file to a .m file

from pathlib import Path

import sys


class Mesh:
    """
    A psgame mesh

    Meshes hold geometry data (vertices & faces,) UV data, color data, etc.
    For simplicity, meshes can only hold triangle faces, not quads

    Note that meshes do not hold texture data, and those needed to be loaded
    separetely
    """

    def __init__(self) -> None:
        self.vertices: list[tuple[float, float, float]] = []
        self.normals: list[tuple[float, float, float]] = []
        self.uvs: list[tuple[float, float]] = []
        self.colors: list[tuple[float, float, float]] = []

        self.faces: list[tuple[int, int, int]] = []
        self.face_normals: list[tuple[int, int, int]] = []
        self.face_textures: list[tuple[int, int, int]] = []
        self.face_types: list[int] = []
        self.face_colors: list[tuple[int, int, int]] = []

    def add_vertex(self, x: float, y: float, z: float) -> None:
        """Adds a vertex to the mesh, as a point in 3D space"""
        self.vertices.append((x, y, z))

    def add_color(self, r: float, g: float, b: float) -> None:
        """Adds a vertex to the mesh, as a point in 3D space"""
        self.colors.append((r, g, b))

    def add_normal(self, x: float, y: float, z: float) -> None:
        """Adds a normal to the mesh, as a vector in 3D space"""
        self.normals.append((x, y, z))

    def add_uv(self, u: float, v: float) -> None:
        """Adds a UV point to the mesh, as a offset into the texture"""
        self.uvs.append((u, v))

    def add_face(self, i0: int, i1: int, i2: int) -> None:
        """Adds a face to the mesh, as indices to the vertex array"""
        self.faces.append((i0, i1, i2))

    def add_face_normal(self, i0: int, i1: int, i2: int) -> None:
        """Adds a face normal to the mesh, as indices to the normal array"""
        self.face_normals.append((i0, i1, i2))

    def add_face_texture(self, i0: int, i1: int, i2: int) -> None:
        """Adds a face texture to the mesh, as indices to the UV array"""
        self.face_textures.append((i0, i1, i2))

    def add_face_type(
        self, is_gouraud_shaded: bool, is_textured: bool, color: tuple[int, int, int]
    ) -> None:
        """Adds a face type to the face array"""
        gouraud_type: int = int(is_gouraud_shaded) << 1
        textured_type: int = int(is_textured)

        face_type: int = gouraud_type | textured_type
        self.face_types.append(face_type)
        self.face_colors.append(color)

    def get_vertex_count(self) -> int:
        """Returns the number of vertices in the mesh"""
        return len(self.vertices)

    def get_face_count(self) -> int:
        """Returns the number of faces in the mesh"""
        return len(self.faces)

    def get_normal_count(self) -> int:
        """Returns the number of normals in the mesh"""
        return len(self.normals)

    def get_uv_count(self) -> int:
        """Returns the number of UVs in the mesh"""
        return len(self.uvs)

    def get_color_count(self) -> int:
        """Returns the number of colors in the mesh"""
        return len(self.colors)

    def parse_face(self, f0: str, f1: str, f2: str) -> None:
        i0, vt0, vn0 = self.__parse_face_component(f0)
        i1, vt1, vn1 = self.__parse_face_component(f1)
        i2, vt2, vn2 = self.__parse_face_component(f2)

        self.add_face(i0, i1, i2)
        self.add_face_texture(vt0, vt1, vt2)
        self.add_face_normal(vn0, vn1, vn2)

    def __parse_face_component(self, f: str) -> tuple[int, int, int]:
        v: int = 0
        vt: int = 0
        vn: int = 0

        match f.count("/"):
            case 0:
                v = int(f)
            case 1:
                v, vt = map(int, f.split("/"))
            case 2:
                s = f.split("/")
                v = int(s[0])
                vn = int(s[2])
                if s[1]:
                    vt = int(s[1])

        return (v - 1, vt - 1, vn - 1)


class Model:
    """
    A psgame model

    Models are merely containers for meshes, which contain the actual geometry
    data
    """

    def __init__(self) -> None:
        self.__mesh: Mesh
        self.meshes: list[Mesh] = []

        self.solid_color: tuple[int, int, int] = (128, 128, 128)
        self.gouraud_shaded: bool = False
        self.textured: bool = False

    def add_mesh(self, mesh: Mesh) -> None:
        """Adds a mesh to the model"""
        self.meshes.append(mesh)

    def get_mesh_count(self) -> int:
        return len(self.meshes)

    def parse(self, filename: Path) -> None:
        with open(filename, "r") as f:
            for line in f.readlines():
                line = line.strip().split()

                if not line or "#" in line[0]:
                    continue

                cmd, args = line[0], line[1:]
                match cmd:
                    case "o":
                        self.begin_mesh()
                    case "v":
                        vertex = map(float, args)
                        self.add_vertex(*vertex)
                    case "vn":
                        vertex_normal = map(float, args)
                        self.add_vertex_normal(*vertex_normal)
                    case "vt":
                        uv = map(float, args)
                        self.add_uv(*uv)
                    case "f":
                        self.add_face(*args)
                    case "s":
                        self.parse_s(args[0])
                    case "usemtl":
                        self.parse_mtl(args[0])

    def begin_mesh(self) -> None:
        self.__mesh: Mesh = Mesh()
        self.meshes.append(self.__mesh)

    def add_vertex(
        self, v0: float, v1: float, v2: float, c0: float, c1: float, c2: float
    ) -> None:
        self.__mesh.add_vertex(v0, v1, v2)
        self.__mesh.add_color(c0, c1, c2)

    def add_vertex_normal(self, vn0: float, vn1: float, vn2: float) -> None:
        self.__mesh.add_normal(vn0, vn1, vn2)

    def add_uv(self, u: float, v: float) -> None:
        self.__mesh.add_uv(u, v)

    def add_face(self, f0: str, f1: str, f2: str) -> None:
        self.__mesh.parse_face(f0, f1, f2)
        self.__mesh.add_face_type(self.gouraud_shaded, self.textured, self.solid_color)

    def parse_s(self, s: str) -> None:
        self.gouraud_shaded = s != "0"

    def parse_mtl(self, mtl: str) -> None:
        self.textured = mtl.startswith("tex")
        if mtl.startswith("#"):
            self.solid_color = self.parse_solid_color(mtl)

    def parse_solid_color(self, color: str) -> tuple[int, int, int]:
        r = int(color[1:3], 16)
        g = int(color[3:5], 16)
        b = int(color[5:7], 16)

        return (r, g, b)

    def save(self, filename: Path) -> None:
        """Saves the model to a file"""
        serializer: ModelSerializer = ModelSerializer(self)
        serializer.save(filename)


class Serializer:
    @staticmethod
    def pad(sz: int = 2) -> bytes:
        """Creates 'sz' bytes of padding"""
        return (0).to_bytes(sz)

    @staticmethod
    def b(b: int, sz: int = 2, signed: bool = True) -> bytes:
        """Turns an integer 'b' into 'sz' bytes"""
        return b.to_bytes(sz, byteorder="little", signed=signed)

    @staticmethod
    def psx_vert(v: float) -> int:
        """Converts a vertex 'v' expressed as floating-point into an integer"""
        return int(v * 64.0)

    @staticmethod
    def psx_color(c: float) -> int:
        """Converts a color 'c' expressed as floating-point into an integer"""
        return int(c * 255.0)

    @staticmethod
    def psx_uv(u: float) -> int:
        """Converts a value 'v' expressed as floating-point into an integer"""
        return int(u * 4096.0)


class MeshSerializer(Serializer):
    def __init__(self, mesh: Mesh) -> None:
        self.mesh: Mesh = mesh

    def save_to(self, f) -> None:
        # Save counts
        vertex_count: int = self.mesh.get_vertex_count()
        f.write(self.b(vertex_count))

        face_count: int = self.mesh.get_face_count()
        f.write(self.b(face_count))

        normal_count: int = self.mesh.get_normal_count()
        f.write(self.b(normal_count))

        uv_count: int = self.mesh.get_uv_count()
        f.write(self.b(uv_count))

        # Save vertices
        for vertex in self.mesh.vertices:
            for v in vertex:
                f.write(self.b(self.psx_vert(v)))

        if vertex_count % 2:
            f.write(self.pad())

        # Save faces
        for face in self.mesh.faces:
            for v in face:
                f.write(self.b(v))

        if face_count % 2:
            f.write(self.pad())

        # Save normals
        for normal in self.mesh.normals:
            for n in normal:
                f.write(self.b(self.psx_vert(n)))

        if normal_count % 2:
            f.write(self.pad())

        # Save face normals
        for normal in self.mesh.face_normals:
            for n in normal:
                f.write(self.b(n))

        if face_count % 2:
            f.write(self.pad())

        # Save UVs
        for uv in self.mesh.uvs:
            u, v = uv
            f.write(self.b(self.psx_uv(u), 2))
            f.write(self.b(4096 - self.psx_uv(v), 2))

        # Save face textures
        for texture in self.mesh.face_textures:
            for t in texture:
                f.write(self.b(t))

        if face_count % 2:
            f.write(self.pad())

        # Save face data
        for i in range(face_count):
            face_type: int = self.mesh.face_types[i]
            f.write(self.b(face_type, 1))

            f0, f1, f2 = self.mesh.faces[i]

            # Gouraud-shaded
            if face_type & 0x2:
                self.write_face_color(f, f0)
                self.write_face_color(f, f1)
                self.write_face_color(f, f2)
                f.write(self.pad())

            # Flat-shaded
            else:
                for c in self.mesh.face_colors[i]:
                    f.write(self.b(c, 1, False))

    def join_color(self, colors: tuple[float, float, float]) -> int:
        r, g, b = map(self.psx_color, colors)
        return r | (g << 8) | (b << 16)

    def write_face_color(self, f, idx: int) -> None:
        for c in map(self.psx_color, self.mesh.colors[idx]):
            f.write(self.b(c, 1, False))


class ModelSerializer(Serializer):
    def __init__(self, model: Model) -> None:
        self.model: Model = model

    def save(self, filename: Path) -> None:
        """Saves a model to a file"""
        with open(filename, "wb") as f:
            mesh_count: int = self.model.get_mesh_count()
            f.write(self.b(mesh_count))
            f.write(self.pad())

            for mesh in self.model.meshes:
                serializer: MeshSerializer = MeshSerializer(mesh)
                serializer.save_to(f)


def main() -> None:
    if len(sys.argv) < 2:
        print("usage: obj_to_m.py [path]")
        return

    path: Path = Path(sys.argv[1])

    m: Model = Model()
    m.parse(path)
    m.save(path.with_suffix(".m"))


main()
