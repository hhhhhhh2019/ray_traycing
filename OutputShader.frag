#version 130

uniform vec2 resolution;
//uniform float startRandom;

const vec3 light = normalize(vec3(0, 0.1, 1));

const float MAX_DIST = 200.;
const int MAX_REFL = 4;

uniform vec2 u_seed1;
uniform vec2 u_seed2;

uniform vec3 origin;
uniform vec2 rotate;

uniform sampler2D backbuffer;
uniform float rate;

uvec4 R_STATE;

uint TausStep(uint z, int S1, int S2, int S3, uint M)
{
	uint b = (((z << S1) ^ z) >> S2);
	return (((z & M) << S3) ^ b);
}

uint LCGStep(uint z, uint A, uint C)
{
	return (A * z + C);
}

vec2 hash22(vec2 p)
{
	p += u_seed1.x;
	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yzx+33.33);
	return fract((p3.xx+p3.yz)*p3.zy);
}

float random()
{
	R_STATE.x = TausStep(R_STATE.x, 13, 19, 12, uint(429496729));
	R_STATE.y = TausStep(R_STATE.y, 2, 25, 4, uint(429496728));
	R_STATE.z = TausStep(R_STATE.z, 3, 11, 17, uint(429496728));
	R_STATE.w = LCGStep(R_STATE.w, uint(1664525), uint(101390422));
	return 2.3283064365387e-10 * float((R_STATE.x ^ R_STATE.y ^ R_STATE.z ^ R_STATE.w));
}

vec3 randomOnSphere() {
	vec3 rand = vec3(random(), random(), random());
	float theta = rand.x * 2.0 * 3.14159265;
	float v = rand.y;
	float phi = acos(2.0 * v - 1.0);
	float r = pow(rand.z, 1.0 / 3.0);
	float x = r * sin(phi) * cos(theta);
	float y = r * sin(phi) * sin(theta);
	float z = r * cos(phi);
	return vec3(x, y, z);
	//return normalize(vec3(random()*2.-1., random()*2.-1., random()*2.-1.));
}


mat2 rot(float a) {
	float s = sin(a);
	float c = cos(a);
	return mat2(c, -s, s, c);
}

vec2 sphInter(in vec3 ro, in vec3 rd, float ra) {
	float b = dot(ro, rd);
	float c = dot(ro, ro) - ra * ra;
	float h = b * b - c;
	if(h < 0.0) return vec2(-1);
	h = sqrt(h);
	return vec2(-b - h, -b + h);
}

vec2 boxInter(in vec3 ro, in vec3 rd, in vec3 rad, out vec3 oN)  {
	vec3 m = 1.0 / rd;
	vec3 n = m * ro;
	vec3 k = abs(m) * rad;
	vec3 t1 = -n - k;
	vec3 t2 = -n + k;
	float tN = max(max(t1.x, t1.y), t1.z);
	float tF = min(min(t2.x, t2.y), t2.z);
	if(tN > tF || tF < 0.0) return vec2(-1);
	oN = -sign(rd) * step(t1.yzx, t1.xyz) * step(t1.zxy, t1.xyz);
	return vec2(tN, tF);
}

float plaInter(in vec3 ro, in vec3 rd, in vec4 p) {
	return -(dot(ro, p.xyz) + p.w) / dot(rd, p.xyz);
}

vec3 getSky(vec3 rd) {
	vec3 col = vec3(0.3, 0.6, 1.0);
	vec3 sun = vec3(0.95, 0.9, 1.0);
	sun *= max(0.0, pow(dot(rd, light), 1024.0));
	col *= max(0.0, dot(light, vec3(0.0, 0.0, sign(light.z))));
	return clamp(sun + col * 0.01, 0.0, 1.0);
}


vec4 trayce(inout vec3 ro, inout vec3 rd) {
	vec2 minIt = vec2(MAX_DIST);
	vec4 col;
	vec3 n;
	vec2 it;

	vec3 bPos = vec3(1,1.99,10);
	vec3 bSize = vec3(1,0.1,1);
	vec3 bN;
	it = boxInter(ro - bPos, rd, bSize, bN);
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = bN;
		col = vec4(1,1,1,-2);
	}

	vec3 spPos = vec3(0,0,10);
	float spRad = 1.;
	it = sphInter(ro - spPos, rd, spRad);
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = normalize(ro + rd * it.x - spPos);
		col = vec4(1,0,0,0);
	}

	spPos = vec3(2,0,13);
	spRad = 1.;
	it = sphInter(ro - spPos, rd, spRad);
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = normalize(ro + rd * it.x - spPos);
		col = vec4(.1,1,.1,0.6);
	}

	bPos = vec3(2,-0.5,7);
	bSize = vec3(0.5);
	it = boxInter(ro - bPos, rd, bSize, bN);
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = bN;
		col = vec4(.1, .1, 1, 0);
	}

	vec4 plNorm = vec4(0, 1, 0, 1);
	it = vec2(plaInter(ro, rd, plNorm));
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = plNorm.xyz;
		col = vec4(0.5, 0.5, 0.5, 0);
	}

	plNorm = vec4(0, -1, 0, 2);
	it = vec2(plaInter(ro, rd, plNorm));
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = plNorm.xyz;
		col = vec4(0.5, 0.5, 0.5, 0);
	}


	plNorm = vec4(0, 0, -1, 14);
	it = vec2(plaInter(ro, rd, plNorm));
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = plNorm.xyz;
		col = vec4(0.5, 0.5, 0.5, 0);
	}

	plNorm = vec4(0, 0, 1, 0);
	it = vec2(plaInter(ro, rd, plNorm));
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = plNorm.xyz;
		col = vec4(0.5, 0.5, 0.5, 0);
	}


	plNorm = vec4(1, 0, 0, 3);
	it = vec2(plaInter(ro, rd, plNorm));
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = plNorm.xyz;
		col = vec4(1, 0.5, 0, 0);
	}

	plNorm = vec4(-1, 0, 0, 3);
	it = vec2(plaInter(ro, rd, plNorm));
	if (it.x > 0. && it.x < minIt.x) {
		minIt = it;
		n = plNorm.xyz;
		col = vec4(0, 0.5, 1, 0);
	}

	if (minIt.x == MAX_DIST) return vec4(.2,.4,1,-1);

	if (col.a == -2.)
		return col;

	vec3 reflected = reflect(rd, n);
	if(col.a < 0.0) {
		float fresnel = 1.0 - abs(dot(-rd, n));
		if(random() - 0.1 < fresnel * fresnel) {
			rd = reflected;
			return col;
		}
		ro += rd * (minIt.y + 0.001);
		rd = refract(rd, n, 1.0 / (1.0 - col.a));
		return col;
	}

	vec3 r = normalize(randomOnSphere());
	vec3 diffuse = normalize(r * dot(r, n));

	ro += rd * (minIt.x - 0.001);
	rd = mix(diffuse, reflected, col.a);

	return col;
}


vec3 render(in vec3 ro, in vec3 rd) {
	vec3 col = vec3(1);

	for (int i = 0; i < MAX_REFL; i++) {
		vec4 nc = trayce(ro, rd);

		col *= nc.rgb;

		if (nc.a == -2.) return col;
	}

	return vec3(0);
}

void main(void) {
	vec2 uv = (gl_FragCoord.xy / resolution - 0.5) * resolution / resolution.y;

	vec2 uvRes = hash22(uv + 1.0) * resolution + resolution;
	R_STATE.x = uint(u_seed1.x + uvRes.x);
	R_STATE.y = uint(u_seed1.y + uvRes.x);
	R_STATE.z = uint(u_seed2.x + uvRes.y);
	R_STATE.w = uint(u_seed2.y + uvRes.y);

	vec3 ro = origin;
	vec3 rd = normalize(vec3(uv, 1));

	rd.yz *= rot(rotate.x);
	rd.xz *= rot(rotate.y);

	vec3 col = vec3(0);
	int samples = 32;
	for(int i = 0; i < samples; i++) {
		col += render(ro, rd);
	}
	col /= samples;

	float white = 10.;
	col *= white * 16.;
	col = (col * (1. + col / white / white)) / (1. + col);

	vec2 tuv = gl_FragCoord.xy / resolution;

	gl_FragColor = vec4(mix(texture2D(backbuffer, tuv).rgb, col, rate), 1.0);
}