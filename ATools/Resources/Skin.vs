vs.1.1
;------------------------------------------------------------------------------
; Constants specified by the app
;    c0-c83  = bone matrix index 0 ~ 28
;    c84-c87 = matViewProj
;    c88-c91 = reserved
;    c92     = light direction
;    c93     = material diffuse color * light diffuse color
;    c94     = material ambient color
;    c95     = fog
;	
;
; Vertex components (as specified in the vertex DECL)
;   v0    = Position
;	v1	  = w1, w2
;	v2.x  = matrix idx
;	v3    = Normal
;	v4    = Texcoords
;------------------------------------------------------------------------------

; Input attributes
dcl_position v0;
dcl_blendweight v1;
dcl_blendindices v2;
dcl_normal v3;
dcl_texcoord0 v4;

; Vertex Pos
mov a0.x, v2.x
m4x3 r0, v0, c[a0.x]
m3x3 r3, v3, c[a0.x]
mul  r1, r0.xyz, v1.x
mul  r4, r3.xyz, v1.x		

mov  a0.x, v2.y
m4x3 r0, v0, c[a0.x]
m3x3 r3, v3, c[a0.x]
mad  r1, r0.xyz, v1.y, r1.xyz
mad  r3, r3.xyz, v1.y, r4.xyz

mov  r1.w, c95.x
m4x4 oPos, r1, c84

; Directional lighting
dp3 r1.x, r3, -c92
lit r1, r1
mul r2, r1.y, c93
add r2, r2, c94
min oD0, r2, c95.x

; Texture coordinates
mov oT0.xy,  v4.xy

; Fog
mov oFog, c95.w