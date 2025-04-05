#version 330

//in vec3 color;
//out vec4 fragment;
//
//uniform float u_time;
//
//void main()
//{
//    fragment = vec4(color, 1.0);
////   gl_FragColor = vec4(color,1.0); -> the default output
//   //gl_FragCoord
//}
#define MARKER_RADIUS 10

in vec3 color;
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

out vec4 fragment;

// Plot a line on Y using a value between 0.0-1.0
float plot(vec2 st) {
    return smoothstep(0.1, 0.0, abs(st.y - st.x));
}


vec3 marker(vec2 center, vec3 color) {
   vec3 out_color = vec3(0.0,0.0,0.0);
   if (length(gl_FragCoord.xy - center) < MARKER_RADIUS) {
      out_color += color;
   }
   return out_color;
}

void main() {
   vec2 st = gl_FragCoord.xy/u_resolution;
//   vec2 st = vec2 (u_time,u_time);
//   vec2 koko = step(vec2(0.2),st);

    float y = st.x;

    vec3 color1 = vec3(y);

    // Plot a line
    float pct = plot(st);
    color1 = (1.0-pct)*color+pct*vec3(0.0,1.0,1.0);
//    color1 -= marker(vec2(u_mouse.x,u_resolution.y - u_mouse.y), vec3(1.0,1.0,1.0));
   color1 = marker(gl_FragCoord.xy, vec3(1.0,1.0,1.0));

//   gl_FragColor = vec4(color,1.0);
   fragment = vec4(color*color1,1.0);
}

