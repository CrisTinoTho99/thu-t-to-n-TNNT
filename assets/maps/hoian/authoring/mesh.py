"""Small indexed mesh writer. Coordinates are metres, +Y up; colours are sRGB."""
from array import array
import json
import math
import struct


def add(a, b): return tuple(x+y for x,y in zip(a,b))
def sub(a, b): return tuple(x-y for x,y in zip(a,b))
def mul(a, s): return tuple(x*s for x in a)
def cross(a,b): return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])
def unit(a): return mul(a,1/max(1e-12,math.sqrt(sum(x*x for x in a))))
def tint(c, k): return tuple(min(255,max(0,round(v*k))) for v in c)
def linear(v):
    v=v/255
    return v/12.92 if v<=.04045 else ((v+.055)/1.055)**2.4


class Mesh:
    def __init__(self):
        self.pos=array('f')
        self.norm=array('f')
        self.col=bytearray()
        self.idx=array('I')
        self.cache={}

    def face(self, pts, color, normals=None):
        n=unit(cross(sub(pts[1],pts[0]),sub(pts[2],pts[0])))
        palette=tuple(round(255*linear(v)) for v in color)+(255,)
        ids=[]
        for i,p in enumerate(pts):
            normal=normals[i] if normals else n
            key=(p,normal,palette)
            idx=self.cache.get(key)
            if idx is None:
                idx=len(self.pos)//3
                self.cache[key]=idx
                self.pos.extend(p); self.norm.extend(normal); self.col.extend(palette)
            ids.append(idx)
        for i in range(1,len(ids)-1): self.idx.extend((ids[0],ids[i],ids[i+1]))

    def box(self,c,size,color):
        x,y,z=c; a,b,d=[v/2 for v in size]
        p=[(x+i*a,y+j*b,z+k*d) for i,j,k in
           [(-1,-1,-1),(1,-1,-1),(1,-1,1),(-1,-1,1),(-1,1,-1),(1,1,-1),(1,1,1),(-1,1,1)]]
        for f in [(0,1,2,3),(4,7,6,5),(0,4,5,1),(1,5,6,2),(2,6,7,3),(3,7,4,0)]:
            self.face([p[i] for i in f],color)

    def rod(self,a,b,r,color,n=10,end=None):
        w=unit(sub(b,a))
        u=unit(cross((0,0,1) if abs(w[2])<.9 else (0,1,0),w))
        v=cross(w,u); end=r if end is None else end
        ns=[add(mul(u,math.cos(i*math.tau/n)),mul(v,math.sin(i*math.tau/n))) for i in range(n)]
        lo=[add(a,mul(s,r)) for s in ns]; hi=[add(b,mul(s,end)) for s in ns]
        for i in range(n):
            j=(i+1)%n
            self.face([lo[i],lo[j],hi[j],hi[i]],color,[ns[i],ns[j],ns[j],ns[i]] if n>5 else None)
        self.face(lo[::-1],color); self.face(hi,color)

    def path(self,points,r,color,n=7):
        for a,b in zip(points,points[1:]):
            if a!=b: self.rod(a,b,r,color,n)


def write_glb(meshes,path):
    buf=bytearray(); views=[]; access=[]; nodes=[]; desc=[]
    def accessor(values,fmt,kind,count,target,**extra):
        buf.extend(bytes(-len(buf)%4))
        raw=values.tobytes() if isinstance(values,array) else values
        views.append(dict(buffer=0,byteOffset=len(buf),byteLength=len(raw),target=target))
        buf.extend(raw)
        access.append(dict(bufferView=len(views)-1,componentType=fmt,type=kind,count=count,**extra))
        return len(access)-1
    for name,m in meshes.items():
        count=len(m.pos)//3
        p=accessor(m.pos,5126,'VEC3',count,34962,min=[min(m.pos[i::3]) for i in range(3)],max=[max(m.pos[i::3]) for i in range(3)])
        n=accessor(m.norm,5126,'VEC3',count,34962)
        c=accessor(m.col,5121,'VEC4',count,34962,normalized=True)
        idx=accessor(m.idx,5125,'SCALAR',len(m.idx),34963)
        nodes.append(dict(name=name,mesh=len(desc)))
        desc.append(dict(name=name,primitives=[dict(attributes=dict(POSITION=p,NORMAL=n,COLOR_0=c),indices=idx,material=0)]))
    doc=dict(asset=dict(version='2.0',generator='Hoi An architectural study'),scene=0,
             scenes=[dict(nodes=list(range(len(nodes))))],nodes=nodes,meshes=desc,
             materials=[dict(doubleSided=True,pbrMetallicRoughness=dict(metallicFactor=0,roughnessFactor=.84))],
             accessors=access,bufferViews=views,buffers=[dict(byteLength=len(buf))])
    data=json.dumps(doc,separators=(',',':')).encode()
    data+=b' '*(-len(data)%4);buf.extend(bytes(-len(buf)%4))
    path.parent.mkdir(exist_ok=True)
    path.write_bytes(struct.pack('<4sIII4s',b'glTF',2,28+len(data)+len(buf),len(data),b'JSON')+data+struct.pack('<I4s',len(buf),b'BIN\0')+buf)
