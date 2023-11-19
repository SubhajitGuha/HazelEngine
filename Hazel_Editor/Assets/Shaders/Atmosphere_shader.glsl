#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 dir;

out vec2 tcord;
out vec3 m_pos;

void main()
{
	gl_Position = pos;
	//tcord = cord.xy;
	m_pos = -dir.xyz;
    tcord = pos.xy*0.5+0.5;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec2 tcord;
in vec3 m_pos;

uniform vec3 RayOrigin;
uniform vec3 view_dir;
uniform vec3 sun_direction;
uniform float densityFalloff;
uniform float ScatteringStrength;
uniform float atmosphere_radius;
uniform float planetRadius;
uniform vec3 ScatteringCoeff;

float time = 0.0;
float cirrus = 0.4;
float cumulus = 0.8;

#define FLT_MAX 3.402823466e+38

  const float Br = 0.0025;
  const float Bm = 0.0003;
  const float g =  0.9800;
  const vec3 nitrogen = vec3(0.650, 0.570, 0.475);
  const vec3 Kr = Br / pow(nitrogen, vec3(4.0));
  const vec3 Km = Bm / pow(nitrogen, vec3(0.84));

  float hash(float n)
  {
    return fract(sin(n) * 43758.5453123);
  }

  float noise(vec3 x)
  {
    vec3 f = fract(x);
    float n = dot(floor(x), vec3(1.0, 157.0, 113.0));
    return mix(mix(mix(hash(n +   0.0), hash(n +   1.0), f.x),
                   mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
               mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                   mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
  }

  const mat3 m = mat3(0.0, 1.60,  1.20, -1.6, 0.72, -0.96, -1.2, -0.96, 1.28);
  float fbm(vec3 p)
  {
    float f = 0.0;
    f += noise(p) / 2; p = m * p * 1.1;
    f += noise(p) / 4; p = m * p * 1.2;
    f += noise(p) / 6; p = m * p * 1.3;
    f += noise(p) / 12; p = m * p * 1.4;
    f += noise(p) / 24;
    return f;
  }

   vec3 ColorCorrection(vec3 incolor)
   {
        //incolor = clamp(incolor,0,1);
        //incolor = pow(incolor, vec3(1.0/2.2)); //Gamma correction
        
        //incolor = clamp(incolor,0,1);
        incolor = vec3(1.0) - exp(-incolor * 1);//exposure
        
        //incolor = clamp(incolor,0,1);
        incolor = mix(vec3(dot(incolor,vec3(0.299,0.587,0.114))), incolor,1.1);//saturation
        
        //incolor = clamp(incolor,0,1);
        incolor = 1.02*(incolor-0.5) + 0.5 + 0.00 ; //contrast
        
        return incolor;
    }

  void main()
  {
    //if (m_pos.y < 0)
      //discard;

    vec3 sun_dir = -sun_direction;
    // Atmosphere Scattering
    float mu = dot(normalize(m_pos), normalize(sun_dir));
    float rayleigh = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu);
    vec3 mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);

    vec3 day_extinction = exp(-exp(-((m_pos.y + sun_dir.y * 4.0) * (exp(-m_pos.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-m_pos.y * 16.0) + 0.1) * Kr / Br) * exp(-m_pos.y * exp(-m_pos.y * 8.0 ) * 4.0) * exp(-m_pos.y * 2.0) * 4.0;
    vec3 night_extinction = vec3(1.0 - exp(sun_dir.y)) * 0.2;
    vec3 extinction = mix(day_extinction, night_extinction, -sun_dir.y * 0.2 + 0.5);
    color.rgb = rayleigh * mie * extinction;

    // Cirrus Clouds
    float density = smoothstep(1.0 - cirrus, 1.0, fbm(m_pos.xyz / m_pos.y * 2.0 + time * 0.05)) * 0.3;
    color.rgb = mix(color.rgb, extinction * 4.0, density * max(m_pos.y, 0.0));

    // Cumulus Clouds
    for (int i = 0; i < 3; i++)// iteration defines the thickness of clouds
    {
      float density = smoothstep(1.0 - cumulus, 1.0, fbm((0.7 + float(i) * 0.01) * m_pos.xyz / m_pos.y + time * 0.3));
      color.rgb = mix(color.rgb, extinction * density * 5.0, min(density, 1.0) * max(m_pos.y, 0.0));
    }

    // Dithering Noise
    color.rgb += noise(m_pos * 1000) * 0.01;
    color = vec4(ColorCorrection(color.rgb),1.0);
  }