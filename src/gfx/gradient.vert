#version 130

uniform vec2 posTopLeft;
uniform vec2 posBottomRight;
uniform int nGradientDirection;
varying float vGradientPos;
out vec4 Color;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	if (nGradientDirection == 0) /* vertical */
		vGradientPos = (gl_Vertex.y - posTopLeft.y) / (posBottomRight.y - posTopLeft.y);
	else if (nGradientDirection == 1) /* horizontal */
		vGradientPos = (gl_Vertex.x - posTopLeft.x) / (posBottomRight.x - posTopLeft.x);
	else /* radial */
	{
		float height = posBottomRight.y - posTopLeft.y;
		float width = posBottomRight.x - posTopLeft.x;
		float M = sqrt(height * height / 4.0 + width * width / 4.0);
		float dy = gl_Vertex.y - (posTopLeft.y + height / 2.0);
		float dx = gl_Vertex.x - (posTopLeft.x + width / 2.0);
		float r = sqrt(dy * dy + dx * dx);
		vGradientPos = r / M;
	}
	Color = vec4(0.0, 0.0, 0.0, 0.0);
}
