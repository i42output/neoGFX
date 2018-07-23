#version 150

precision mediump float;
uniform float posViewportTop;
uniform vec2 posTopLeft;
uniform vec2 posBottomRight;
uniform int nGradientDirection;
uniform int nGradientStartFrom;
uniform float radGradientAngle;
uniform int nGradientSize;
uniform int nGradientShape;
uniform int nStopCount;
uniform vec2 exponents;
uniform vec2 posGradientCentre;
uniform int nFilterSize;
uniform sampler2DRect texStopPositions;
uniform sampler2DRect texStopColours;
uniform sampler2DRect texFilter;
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

float normalized_ellipse_radius(float angle)
{
	float x = pow(abs(cos(angle)), 2.0 / exponents.x) * sign(cos(angle));
	float y = pow(abs(sin(angle)), 2.0 / exponents.y) * sign(sin(angle));
	return sqrt(x * x + y * y);
}

float ellipse_radius(vec2 ab, vec2 centre, vec2 pt)
{
	vec2 d = pt - centre;
	float angle = atan(d.y, d.x);
	float nr = normalized_ellipse_radius(angle);
	float x = nr * cos(angle) * ab.x;
	float y = nr * sin(angle) * ab.y;
	return sqrt(x * x + y * y);
}

float distance_to_line(vec2 pt1, vec2 pt2, vec2 testPt)
{
	vec2 lineDir = pt2 - pt1;
	vec2 perpDir = vec2(lineDir.y, -lineDir.x);
	vec2 dirToPt1 = pt1 - testPt;
	return abs(dot(normalize(perpDir), dirToPt1)); 
}

vec4 colour_at(vec2 viewPos)
{
	vec2 s = posBottomRight - posTopLeft;
	vec2 pos = viewPos - posTopLeft;
	pos.x = max(min(pos.x, s.x - 1.0), 0.0);
	pos.y = max(min(pos.y, s.y - 1.0), 0.0);
	float gradientPos;
	if (nGradientDirection == 0) /* vertical */
		gradientPos = pos.y / s.y;
	else if (nGradientDirection == 1) /* horizontal */
		gradientPos = pos.x / s.x;
	else if (nGradientDirection == 2) /* diagonal */
	{
		vec2 centre = s / 2.0;
		float angle;
		switch (nGradientStartFrom)
		{
		case 0:
			angle = atan(centre.y, -centre.x);
			break;
		case 1:
			angle = atan(-centre.y, -centre.x);
			break;
		case 2:
			angle = atan(-centre.y, centre.x);
			break;
		case 3:
			angle = atan(centre.y, centre.x);
			break;
		default:
			angle = radGradientAngle;
			break;
		}
		pos.y = s.y - pos.y;
		pos = pos - centre;
		mat2 rot = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
		pos = rot * pos;
		pos = pos + centre;
		gradientPos = pos.y / s.y;
	}
	else if (nGradientDirection == 3) /* rectangular */
	{
		float vert = pos.y / s.y;
		if (vert > 0.5)
			vert = 1.0 - vert;
		float horz = pos.x / s.x;
		if (horz > 0.5)
			horz = 1.0 - horz;
		gradientPos = min(vert, horz) * 2.0;
	}
	else /* radial */
	{
		vec2 ab = s / 2.0;
		pos -= ab;
		vec2 centre = ab * posGradientCentre;
		float d = distance(centre, pos);
		vec2 c1 = posTopLeft - posTopLeft - ab;
		vec2 c2 = vec2(posTopLeft.x, posBottomRight.y) - posTopLeft - ab;
		vec2 c3 = posBottomRight - posTopLeft - ab;
		vec2 c4 = vec2(posBottomRight.x, posTopLeft.y) - posTopLeft - ab;
		vec2 cc = c1;
		if (distance(centre, c2) < distance(centre, cc))
			cc = c2;
		if (distance(centre, c3) < distance(centre, cc))
			cc = c3;
		if (distance(centre, c4) < distance(centre, cc))
			cc = c4; 
		vec2 fc = c1;
		if (distance(centre, c2) > distance(centre, fc))
			fc = c2;
		if (distance(centre, c3) > distance(centre, fc))
			fc = c3;
		if (distance(centre, c4) > distance(centre, fc))
			fc = c4;
		vec2 cs = vec2(min(abs(-ab.x + centre.x), abs(ab.x + centre.x)), min(abs(-ab.y + centre.y), abs(ab.y + centre.y)));
		vec2 fs = vec2(max(abs(-ab.x + centre.x), abs(ab.x + centre.x)), max(abs(-ab.y + centre.y), abs(ab.y + centre.y)));
		float r;
		if (nGradientShape == 0) // Ellipse
		{
			switch(nGradientSize)
			{
			default:
			case 0: // ClosestSide
				r = ellipse_radius(cs, centre, pos);
				break;
			case 1: // FarthestSide
				r = ellipse_radius(fs, centre, pos);
				break;
			case 2: // ClosestCorner
				r = ellipse_radius(ab, vec2(abs(centre.x - cc.x), abs(centre.y - cc.y)), pos);
				break;
			case 3: // FarthestCorner
				r = ellipse_radius(ab, vec2(abs(centre.x - fc.x), abs(centre.y - fc.y)), pos);
				break;
			}
		}
		else if (nGradientShape == 1) // Circle
		{
			switch(nGradientSize)
			{
			default:
			case 0:
				r = min(cs.x, cs.y);
				break;
			case 1:
				r = max(fs.x, fs.y);
				break;
			case 2:
				r = distance(cc, centre);
				break;
			case 3:
				r = distance(fc, centre);
				break;
			}
		}
		if (d < r)
			gradientPos = d/r;
		else
			gradientPos = 1.0;
	}
	return gradient_colour(gradientPos);
}

int to_1d(int x, int y, int width)
{
	if (x < 0)
		x = 0;
	if (x > width - 1)
		x = width - 1;
	if (y < 0)
		y = 0;
	if (y > width - 1)
		y = width - 1;
	return x + (y * width);
}

void main()
{
	vec2 viewPos = gl_FragCoord.xy;
	viewPos.y = posViewportTop - viewPos.y;
	int d = nFilterSize / 2;
	if (texelFetch(texFilter, ivec2(d, d)).r == 1.0)
	{
		FragColor = colour_at(viewPos);
	}
	else
	{
		vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
		for (int fy = -d; fy <= d; ++fy)
		{
			for (int fx = -d; fx <= d; ++fx)
			{
				sum += (colour_at(viewPos + vec2(fx, fy)) * texelFetch(texFilter, ivec2(fx + d, fy + d)).r);
			}
		}
		FragColor = sum;
	}
}