#version 330 core
in vec3 world,surfaceNormal,pigment,sunCoord;
out vec4 result;
uniform sampler2D shadowDepth, mirrorColor;
uniform vec3 eye,sun,frame;
uniform int material, mirrored;
float hash(vec3 p){p=fract(p*.1031);p+=dot(p,p.yzx+33.33);return fract((p.x+p.y)*p.z);}
float noise(vec3 p){
    vec3 i=floor(p),f=fract(p); f=f*f*(3.-2.*f);
    return mix(mix(mix(hash(i),hash(i+vec3(1,0,0)),f.x),mix(hash(i+vec3(0,1,0)),hash(i+vec3(1,1,0)),f.x),f.y),
               mix(mix(hash(i+vec3(0,0,1)),hash(i+vec3(1,0,1)),f.x),mix(hash(i+vec3(0,1,1)),hash(i+vec3(1)),f.x),f.y),f.z);
}
float visibility(vec3 n){
    if(any(lessThan(sunCoord,vec3(0)))||any(greaterThan(sunCoord,vec3(1))))return 1.;
    float bias=max(.00012,.00060*(1.-max(0.,dot(n,sun))));
    float sum=0.;vec2 texel=1./vec2(textureSize(shadowDepth,0));
    for(int i=-1;i<=1;i++)for(int j=-1;j<=1;j++){
        sum+=step(sunCoord.z-bias,texture(shadowDepth,sunCoord.xy+vec2(i,j)*texel*1.35).r);
    }
    return sum/9.;
}
void main(){
    if(mirrored==1&&world.y<-.23)discard;
    vec3 n=normalize(surfaceNormal),v=normalize(eye-world);
    if(dot(n,v)<0.)n=-n;
    float shade=visibility(n), ndl=max(0.,dot(n,sun));
    if(material==13){
        vec2 uv=gl_FragCoord.xy/frame.xy;
        float a=sin(world.x*3.1+world.z*3.7+frame.z*.85);
        float b=sin(world.z*8.1-world.x*1.4-frame.z*1.1+.9*a);
        vec2 ripple=vec2(a*.0018+b*.0007,b*.0028+a*.0011);
        // Distance to the nearest pair of cutwaters, mirrored across the channel.
        float px=abs(world.x);
        float nearest=px<2.37?1.52:(px<4.46?3.22:5.70);
        vec2 fromTip=vec2(px-nearest,abs(world.z)-2.22);
        float dist=length(fromTip);
        float wake=sin(13.*dist-frame.z*2.2)*exp(-1.65*dist)
                  *smoothstep(-.30,.14,fromTip.y);
        vec2 direction=sign(world.xz)*fromTip/max(dist,.02);
        ripple+=direction*wake*.009;
        vec3 reflection=texture(mirrorColor,clamp(uv+ripple,.001,.999)).rgb;
        float fresnel=.20+.64*pow(1.-abs(v.y),3.);
        vec3 base=vec3(.20,.32,.28)*(.71+.29*shade);
        result=vec4(mix(base,reflection,fresnel)+.010*(a+b)+.021*wake,1);return;
    }
    vec3 albedo=pigment;
    float rough=noise(world*9.)*.55+noise(world*33.)*.30+noise(world*92.)*.15;
    if(material==2||material==3||material==4||material==8){
        vec3 p=world;
        float deckY=1.16+.34*max(0.,1.-world.x*world.x/81.);
        float height=world.y-deckY;
        float bent=abs(fract((world.x+9.)/(18./7.)+.5)-.5)*(18./7.);
        if(material==2 && height>-.115) p=world.zyx;
        if(material==3 && bent<.10 && abs(height-2.48)<.09) p=world.zyx;
        else if(material==3 && bent<.12 && abs(abs(world.z)-1.42)<.12 && height<2.17) p=world.yxz;
        if(material==4 && height>.24 && height<.61) p=world.yxz;
        float wander=noise(p*vec3(.8,3.,3.));
        float grain=noise(p*vec3(1.35,51.,51.)+1.7*wander);
        albedo*=.77+.41*grain+.08*rough;
    }else if(material==1||material==11){
        albedo*=.77+.43*rough;
        float block=hash(vec3(floor(abs(world.x)+.5),floor((world.z+2.10)/.84),0.));
        float dampHeight=-.035+.15*block+.09*noise(world*3.4)+.035*rough;
        float wet=1.-smoothstep(dampHeight-.07,dampHeight+.04,world.y);
        albedo*=mix(vec3(1.),vec3(.31,.43,.33),wet);
    }else albedo*=.91+.18*rough;
    float access=mix(.82,1.,clamp((abs(world.z)-1.2)*.6,0.,1.));
    vec3 ambient=mix(vec3(.40,.36,.34),vec3(.55,.62,.71),n.y*.5+.5)*access;
    vec3 radiance=albedo*(ambient+vec3(1.12,.91,.68)*ndl*shade);
    if(material==7||material==9||material==4){
        float railY=1.16+.34*max(0.,1.-world.x*world.x/81.)+.85;
        float top=smoothstep(.12,.82,n.y)*smoothstep(-.08,-.025,world.y-railY);
        float polish=material==4?mix(.035,.46,top):.68;
        float shine=pow(max(0.,dot(n,normalize(v+sun))),material==7?74.:mix(19.,45.,top));
        radiance+=vec3(.45,.36,.21)*shine*shade*polish;
        if(material==4) radiance+=vec3(.036,.030,.020)*top*pow(1.-abs(dot(v,n)),2.);
    }
    vec3 rgb=pow(max(vec3(0),radiance),vec3(1./2.2));
    result=vec4(mix(rgb,vec3(.84,.85,.80),smoothstep(24.,100.,length(eye-world))*.62),1);
}
