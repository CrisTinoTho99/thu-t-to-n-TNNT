"""Close-range geometry for the Hội An walkthrough. All dimensions are metres."""
import math
from pathlib import Path

from mesh import tint


def deck_plank(mesh, x, y, width, color):
    # The section runs across the bridge. The two narrow shoulders catch grazing light.
    a = width / 2
    section = [(-a, -.11), (a, -.11), (a, -.006), (a-.006, 0),
               (-a+.006, 0), (-a, -.006)]
    ends = [[(x+dx, y+dy, z) for dx, dy in section] for z in (-1.59, 1.59)]
    mesh.face(ends[0][::-1], color)
    mesh.face(ends[1], color)
    for i in range(len(section)):
        j = (i+1) % len(section)
        mesh.face([ends[0][i], ends[0][j], ends[1][j], ends[1][i]], color)


def central_crest(mesh, point, half, crown):
    lime, blue, porcelain = (190, 177, 151), (38, 76, 127), (223, 222, 207)
    def panel_point(x, depth, h):
        return point(x, depth, crown(x) + h)

    # Continuous curved sill and cap tie the three recessed fields together.
    for i in range(36):
        a, b = -half + 2*half*i/36, -half + 2*half*(i+1)/36
        for h, radius, color in ((.12,.046,lime), (.41,.047,lime), (.447,.018,porcelain)):
            mesh.rod(panel_point(a,0,h), panel_point(b,0,h), radius, color, 8)
        if abs((a+b)/2) > .73:
            for depth in (-.034,.034):
                mesh.face([panel_point(a,depth,.13),panel_point(b,depth,.13),
                           panel_point(b,depth,.40),panel_point(a,depth,.40)], lime)
    for x in (-half,-.75,.75,half):
        mesh.rod(panel_point(x,0,.12),panel_point(x,0,.42),.045,porcelain,8)

    # Open square cores in the round coin tiles admit sky through the centre field.
    for x in (-.54, -.27, 0, .27, .54):
        for i in range(24):
            angles = [math.tau*k/24 for k in (i,i+1)]
            def ring(angle, inner, depth):
                c,s = math.cos(angle), math.sin(angle)
                r = .052/max(abs(c),abs(s)) if inner else .133
                return panel_point(x+r*c,depth,.267+r*s)
            for depth in (-.04,.04):
                mesh.face([ring(angles[0],True,depth), ring(angles[1],True,depth),
                           ring(angles[1],False,depth),ring(angles[0],False,depth)], (160,110,76))
            for inner in (True,False):
                mesh.face([ring(angles[0],inner,-.04),ring(angles[1],inner,-.04),
                           ring(angles[1],inner,.04),ring(angles[0],inner,.04)], (143,92,65))

    # Shallow dishes on both faces, with a cobalt rim and a radial painted motif.
    for x in (-1.83,-1.24,1.24,1.83):
        for side in (-1,1):
            for i in range(32):
                angles = [math.tau*k/32 for k in (i,i+1)]
                for r0,r1,d0,d1,c in ((0,.044,.055,.056,blue),(.044,.112,.056,.069,porcelain),
                                        (.112,.127,.069,.078,blue),(.127,.14,.078,.075,porcelain)):
                    def p(r,d,t):
                        return panel_point(x+r*math.cos(t),side*d,.27+r*math.sin(t))
                    pts = [p(r1,d1,angles[0]),p(r1,d1,angles[1]),p(r0,d0,angles[1])]
                    if r0: pts.append(p(r0,d0,angles[0]))
                    mesh.face(pts, blue if r0==.044 and i%4==0 else c)
    # Sun and curling lime clouds above the raised middle panel.
    mesh.rod(panel_point(0,-.065,.61), panel_point(0,.065,.61), .128, (185,113,63), 28)
    for sign in (-1,1):
        for x,h,span in ((.12,.48,.46),(.29,.47,.27)):
            pts = [panel_point(sign*(x+span*i/15),0,h+.12*math.sin(i*math.pi/10)+.07*i/15)
                   for i in range(16)]
            mesh.path(pts,.032,porcelain,8)


def add_shophouse(mesh, x, z, facing, width, height, plaster, turn=False):
    # Local +depth points out of the shop, towards the alley.
    def p(u,y,d):
        return (x + facing*d,y,z + u) if turn else (x + u,y,z + facing*d)

    def box(u,y,d,w,h,depth,color):
        mesh.box(p(u,y,d), (depth,h,w) if turn else (w,h,depth), color)

    wood, dark, clay = (99,72,48), (60,49,38), (138,89,64)
    ground, opening = 1.15, width*.45
    box(0, ground+height/2, -2.75, width,height,4.0,plaster)
    for side in (-1,1):
        jamb = (width-opening)/2
        box(side*(opening+jamb)/2, ground+height/2,-.38,jamb,height,.78,plaster)
        box(side*(opening/2+.035),ground+1.12,-.02,.10,2.24,.14,wood)
    box(0,ground+(height+2.2)/2,-.38,opening,height-2.2,.78,plaster)
    for i in range(6):
        u = opening*((i+.5)/6-.5)
        box(u,ground+1.18,-.72,opening/6-.018,2.15,.085,tint(wood, .83+.05*(i%3)))
        box(u,ground+1.62,-.665,opening/6-.045,.54,.032,dark)
    # The gable ends face an observer on the bridge before the shopfronts turn into view.
    for side in (-1,1):
        u = side*(width/2+.012)
        mesh.face([p(u,ground+height,0),p(u,ground+height,-4.75),
                   p(u,ground+height+1.04,-2.3)],tint(plaster,.95))
        box(u,ground+.24,-2.34,.055,.45,4.72,tint(plaster,.73))
        box(u,ground+1.78,-2.50,.075,1.48,.98,dark)
        for d in (-2.74,-2.26):
            box(u+side*.046,ground+1.78,d,.09,1.35,.43,wood)
            for j in range(7):
                box(u+side*.096,ground+1.49+j*.105,d,.04,.039,.36,tint(wood,.74))
        box(u+side*.05,ground+1.02,-2.50,.17,.09,1.12,wood)
    box(0,ground+.08,.42,opening+.5,.16,.95,(153,145,126))
    box(0,ground+.18,-.15,opening+.18,.20,.83,(175,163,140))
    for side in (-1,1):
        u = side*(width/2-.18)
        box(u,ground+1.15,.94,.095,2.3,.095,wood)
        mesh.rod(p(u,ground+1.86,.94),p(u,ground+2.34,.53),.053,wood,4)
    def roof(lo, hi, d0, d1, over):
        # A solid edge gives the clay overhang thickness in silhouette.
        w = width*.5+over
        a,b,c,e = p(-w,lo,d0),p(w,lo,d0),p(w,hi,d1),p(-w,hi,d1)
        mesh.face([a,b,c,e],clay)
        mesh.face([a,b,p(w,lo-.085,d0),p(-w,lo-.085,d0)], tint(clay,.72))
        for j in range(round(width/.22)+1):
            u = -w+2*w*j/round(width/.22)
            mesh.rod(p(u,lo+.018,d0),p(u,hi+.018,d1),.036,tint(clay,1.09),6)
    roof(ground+2.35,ground+2.75,1.20,-.08,.10)
    roof(ground+height,ground+height+1.05,.26,-2.3,.17)
    roof(ground+height,ground+height+1.05,-4.98,-2.3,.17)
    # A timber grille over the entrance is visible through the opposite portal.
    box(0,ground+2.31,-.02,opening,.20,.06,dark)
    for j in range(11):
        box(opening*(j-5)/11,ground+2.31,.024,.035,.18,.04,wood)


def landing(mesh):
    # Six treads descend from the east quay to a low platform by the water.
    for i in range(6):
        x, top = 4.32+i*.34, .20+i*.16
        for j in range(3):
            mesh.box((x,(top-.32)/2,5.40+.75*j),(.34,top+.32,.745),
                     tint((157,149,127),.91+.025*((i+2*j)%5)))
    for j in range(3):
        mesh.box((3.78,-.06,5.40+.75*j),(.77,.52,.745),tint((148,143,123),1+.018*j))


def plaque_letters(mesh, cy, z, gold):
    # Coverage traced from the photographed panel, in physical left-to-right order.
    with Path(__file__).with_name('plaque-lettering.pgm').open('rb') as f:
        if f.readline().strip()!=b'P5': raise ValueError('Expected grayscale plaque coverage')
        width,height = map(int,f.readline().split())
        if f.readline().strip()!=b'255': raise ValueError('Expected 8-bit plaque coverage')
        pixels = f.read()

    def clipped(vertices, threshold, above):
        out = []
        prev = vertices[-1]
        for cur in vertices:
            if (prev[2] >= threshold) != (cur[2] >= threshold):
                t = (threshold-prev[2])/(cur[2]-prev[2])
                out.append((prev[0]+t*(cur[0]-prev[0]),prev[1]+t*(cur[1]-prev[1]),threshold))
            if (cur[2]>=threshold)==above: out.append(cur)
            prev = cur
        return out

    # Clip the bevel against each triangular cell. Interior runs share one flat quad.
    low,high = 56.5,189.5
    def point(v):
        x,y,c = v
        t = min(1,max(0,(c-low)/(high-low)))
        return ((x/(width-1)-.5)*.996+.017, cy+(.5-y/(height-1))*.346, z+.038+t*.009)

    for y in range(height-1):
        x = 0
        while x < width-1:
            def coverage(i,j): return pixels[j*width+i]
            corners = [(i,j,coverage(i,j)) for i,j in ((x,y+1),(x+1,y+1),(x+1,y),(x,y))]
            if min(v[2] for v in corners)>=high:
                end=x+1
                while end+1<width and min(coverage(end+1,y),coverage(end+1,y+1))>=high:
                    end+=1
                mesh.face([point((i,j,255)) for i,j in ((x,y+1),(end,y+1),(end,y),(x,y))],gold)
                x=end
                continue
            if max(v[2] for v in corners)>low:
                for tri in (corners[:3],[corners[0],corners[2],corners[3]]):
                    face = clipped(tri,high,True)
                    bevel = clipped(tri,low,True)
                    if bevel: bevel=clipped(bevel,high,False)
                    for polygon,color in ((face,gold),(bevel,tint(gold,.92))):
                        if len(polygon)>=3: mesh.face([point(v) for v in polygon],color)
            x+=1
