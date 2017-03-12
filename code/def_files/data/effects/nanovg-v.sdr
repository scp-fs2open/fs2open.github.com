
layout(std140) uniform NanoVGUniformData {
    mat3 scissorMat;

    mat3 paintMat;

    vec4 innerCol;

    vec4 outerCol;

    vec2 scissorExt;
    vec2 scissorScale;

    vec2 extent;
    float radius;
    float feather;

    float strokeMult;
    float strokeThr;
    int texType;
    int type;

    vec2 viewSize;
    int texArrayIndex;
};

in vec2 vertPosition;
in vec2 vertTexCoord;

out vec2 ftcoord;
out vec2 fpos;

void main(void) {
	ftcoord = vertTexCoord;
	fpos = vertPosition;
	gl_Position = vec4(2.0*vertPosition.x/viewSize.x - 1.0, 1.0 - 2.0*vertPosition.y/viewSize.y, 0, 1);
}
