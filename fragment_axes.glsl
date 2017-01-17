//#version 120

uniform float time;
uniform vec2 resolution;
uniform int eye;
uniform vec3 ray;
uniform vec3 origin;
uniform vec3 up;


void main( void )
{
	vec3 orth = cross(ray, up);
	vec2 normXY = gl_FragCoord.xy / resolution.xy - 0.5;
	float overlapOffset = 0.15;
	normXY.x += eye == 0 ? -overlapOffset : overlapOffset;// 70% eye overlap
	normXY *= 2.3835; // 100 degree FoV (2.0*tan(50))
	
	vec3 camera = origin;
	float eyeoff = 0.063/2.0;
	float offset = eye == 0 ? -eyeoff : eyeoff;
	camera = camera + offset * orth;

	vec3 camRay = ray + orth * normXY.x + up * normXY.y;
	vec3 normRay = normalize(camRay);

	vec3 fragRayOri = camera;
	vec3 fragRayDir = normRay;
    gl_FragColor = vec4(
        abs(fragRayDir.x) < 0.01 ? 1.0 : 0.0,
        abs(fragRayDir.y) < 0.01 ? 1.0 : 0.0,
        abs(fragRayDir.z) < 0.01 ? 1.0 : 0.0,
        1.0);
}