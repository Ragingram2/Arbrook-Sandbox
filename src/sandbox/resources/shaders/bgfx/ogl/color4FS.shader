FSH          :  #version 420
out vec4 bgfx_FragColor;
#define gl_FragColor bgfx_FragColor
#define texture2D          texture
#define texture2DLod       textureLod
#define texture2DGrad      textureGrad
#define texture2DProjLod   textureProjLod
#define texture2DProjGrad  textureProjGrad
#define textureCubeLod     textureLod
#define textureCubeGrad    textureGrad
#define texture3D          texture
#define texture2DLodOffset textureLodOffset
#define attribute in
#define varying in
#define bgfxShadow2D(_sampler, _coord)     vec4_splat(texture(_sampler, _coord))
#define bgfxShadow2DProj(_sampler, _coord) vec4_splat(textureProj(_sampler, _coord))
vec3 instMul(vec3 _vec, mat3 _mtx) { return ( (_vec) * (_mtx) ); }
vec3 instMul(mat3 _mtx, vec3 _vec) { return ( (_mtx) * (_vec) ); }
vec4 instMul(vec4 _vec, mat4 _mtx) { return ( (_vec) * (_mtx) ); }
vec4 instMul(mat4 _mtx, vec4 _vec) { return ( (_mtx) * (_vec) ); }
float rcp(float _a) { return 1.0/_a; }
vec2 rcp(vec2 _a) { return vec2(1.0)/_a; }
vec3 rcp(vec3 _a) { return vec3(1.0)/_a; }
vec4 rcp(vec4 _a) { return vec4(1.0)/_a; }
vec2 vec2_splat(float _x) { return vec2(_x, _x); }
vec3 vec3_splat(float _x) { return vec3(_x, _x, _x); }
vec4 vec4_splat(float _x) { return vec4(_x, _x, _x, _x); }
uvec2 uvec2_splat(uint _x) { return uvec2(_x, _x); }
uvec3 uvec3_splat(uint _x) { return uvec3(_x, _x, _x); }
uvec4 uvec4_splat(uint _x) { return uvec4(_x, _x, _x, _x); }
mat4 mtxFromRows(vec4 _0, vec4 _1, vec4 _2, vec4 _3)
{
return transpose(mat4(_0, _1, _2, _3) );
}
mat4 mtxFromCols(vec4 _0, vec4 _1, vec4 _2, vec4 _3)
{
return mat4(_0, _1, _2, _3);
}
mat3 mtxFromRows(vec3 _0, vec3 _1, vec3 _2)
{
return transpose(mat3(_0, _1, _2) );
}
mat3 mtxFromCols(vec3 _0, vec3 _1, vec3 _2)
{
return mat3(_0, _1, _2);
}
uniform vec4 u_viewRect;
uniform vec4 u_viewTexel;
uniform mat4 u_view;
uniform mat4 u_invView;
uniform mat4 u_proj;
uniform mat4 u_invProj;
uniform mat4 u_viewProj;
uniform mat4 u_invViewProj;
uniform mat4 u_model[32];
uniform mat4 u_modelView;
uniform mat4 u_modelViewProj;
uniform vec4 u_alphaRef4;
void main()
{
gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
 