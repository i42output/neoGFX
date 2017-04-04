#version 150

uniform float posViewportTop;
uniform vec2 posTopLeft;
uniform vec2 posBottomRight;
uniform int nGradientDirection;
uniform int nGradientSize;
uniform int nGradientShape;
uniform int nStopCount;
uniform sampler2DRect texStopPositions;
uniform sampler2DRect texStopColours;
in vec4 Color;
out vec4 FragColor;

vec4 gradient_colour(in float n)
{
	int l = 0;
	int r = nStopCount - 1;
	int found = -1;
	float pos = 0.0;
	if (n < 0.0)
		n = 0.0;
	if (n > 1.0)
		n = 1.0;
	while(found == -1)
	{
		int m = (l + r) / 2;
		pos = texelFetch(texStopPositions, ivec2(m, 0)).r;
		if (l > r)
			found = r;
		else
		{
			if (pos < n)
				l = m + 1;
			else if (pos > n)
				r = m - 1;
			else
				found = m;
		}
	}
	if (pos >= n && found != 0)
		--found;
	float firstPos = texelFetch(texStopPositions, ivec2(found, 0)).r;
	float secondPos = texelFetch(texStopPositions, ivec2(found + 1, 0)).r;
	vec4 firstColour = texelFetch(texStopColours, ivec2(found, 0));
	vec4 secondColour = texelFetch(texStopColours, ivec2(found + 1, 0));
	return mix(firstColour, secondColour, (n - firstPos) / (secondPos - firstPos));
}

float ellipse_radius(float cx, float cy, float angle)
{
	return cx * cy / sqrt(cx * cx * sin(angle) * sin(angle) + cy * cy * cos(angle) * cos(angle));
}

void main()
{
	vec4 pos = gl_FragCoord;
	pos.y = posViewportTop - pos.y;
	float gradientPos;
	if (nGradientDirection == 0) /* vertical */
		gradientPos = (pos.y - posTopLeft.y) / (posBottomRight.y - posTopLeft.y);
	else if (nGradientDirection == 1) /* horizontal */
		gradientPos = (pos.x - posTopLeft.x) / (posBottomRight.x - posTopLeft.x);
	else if (nGradientDirection == 2) /* diagonal - todo */
		gradientPos = (pos.x - posTopLeft.x) / (posBottomRight.x - posTopLeft.x);
	else /* radial */
	{
	    float x = pos.x - posTopLeft.x;
		float y = pos.y - posTopLeft.y;
		float cx = (posBottomRight.x - posTopLeft.x);
		float cy = (posBottomRight.y - posTopLeft.y);
		float centreX = cx / 2.0;
		float centreY = cy / 2.0;
		float dx = x - centreX;
		float dy = y - centreY;
		float d = sqrt(dx * dx + dy * dy);
		float dnc = min(sqrt(centreX * centreX + centreY * centreY), sqrt(centreX * centreX + centreY * centreY));
		float rnc = ellipse_radius(centreX, centreY, atan(centreY, centreX));
		float dfc = max(sqrt(centreX * centreX + centreY * centreY), sqrt(centreX * centreX + centreY * centreY));
		float rfc = ellipse_radius(centreX, centreY, atan(centreY, centreX));
		float r;
		if (nGradientShape == 0)
		{
			r = ellipse_radius(centreX, centreY, atan(dy, dx));
			switch(nGradientSize)
			{
			default:
			case 0:
			case 1:
				break;
			case 2:
				r = r * dnc / rnc;
				break;
			case 3:
				r = r * dfc / rfc;
				break;
			}
		}
		else
		{
			switch(nGradientSize)
			{
			default:
			case 0:
				r = min(centreX, centreY);
				break;
			case 1:
				r = max(centreX, centreY);
				break;
			case 2:
				r = dnc;
				break;
			case 3:
				r = dfc;
				break;
			}
		}
		if (d < r)
			gradientPos = d/r;
		else
			gradientPos = 1.0;
	}

	FragColor = gradient_colour(gradientPos);
}
