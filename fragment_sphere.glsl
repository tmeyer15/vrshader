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
	normXY.x += eye == 0 ? -overlapOffset : overlapOffset; // 70% eye overlap
	normXY *= 2.3835; // 100 degree FoV (2.0*tan(50))

	vec3 camera = origin;
	float eyeoff = 0.063/2.0;
	float offset = eye == 0 ? -eyeoff : eyeoff;
	camera = camera + offset * orth;

	vec3 position = camera + ray + orth * normXY.x + up * normXY.y;

	vec3 camRay = position - camera;
	vec3 normRay = normalize(camRay);

	vec3 sphere = vec3(1.25 * sin(time/0.95), 1.5 + 0.5 * sin(time/1.23), -2.0 + sin(time/0.56));
	
	vec3 light = vec3(2.0, 2.0, 2.0);
	float radius = 0.1;

	float b = dot(normRay, camera - sphere);
	float c = pow(length(camera - sphere), 2.0) - pow(radius, 2.0);
	
	float d1 = -b + sqrt(pow(b, 2.0) - c);
	float d2 = -b - sqrt(pow(b, 2.0) - c);

	float d = d1 < d2 && d1 > 0.0 ? d1 : 0.0;
	d = d2 <= d1 && d2 > 0.0 ? d2 : 0.0;
	
	vec3 intersectionPoint = normRay * d + camera;

	vec3 pointToLight = light - intersectionPoint;
	vec3 sphereToPoint = intersectionPoint - sphere;
	vec3 reflectVect = reflect( -pointToLight, normalize(-sphereToPoint));

	float ambient = 1.0;
	ambient = (d > 0.0 && ambient > 0.0 ? ambient: 0.0);
	
	float diffuse = max(0.0, dot(pointToLight, sphereToPoint)) / (length(pointToLight) * length(sphereToPoint));
	diffuse = (d > 0.0 && diffuse > 0.0 ? diffuse: 0.0);
	
	float spec = pow(max(0.0, dot(-normRay, normalize(reflectVect))), 20.0);
	spec = (d > 0.0 && spec > 0.0 && diffuse > 0.0 ? spec: 0.0);
	
	float color = 0.0;
	
	color = 0.15 * ambient + 0.6 * diffuse + 0.8 * spec;
	float otherColor = 0.2 * diffuse + 0.8 * spec;
	gl_FragColor = vec4( color, otherColor, otherColor, 1.0 );
}