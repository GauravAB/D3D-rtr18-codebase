float pyramid_vertices[] =
{
    // triangle of front side
    0.0f, 1.0f, 0.0f,    // front-top
    -1.0f, -1.0f, 1.0f,  // front-left
    1.0f, -1.0f, 1.0f,   // front-right
    
    // triangle of right side
    0.0f, 1.0f, 0.0f,    // right-top
    1.0f, -1.0f, 1.0f,   // right-left
    1.0f, -1.0f, -1.0f,  // right-right
    
    // triangle of back side
    0.0f, 1.0f, 0.0f,    // back-top
    1.0f, -1.0f, -1.0f,  // back-right
    -1.0f, -1.0f, -1.0f, // back-left
    
    // triangle of left side
    0.0f, 1.0f, 0.0f,    // left-top
    -1.0f, -1.0f, -1.0f, // left-left
    -1.0f, -1.0f, 1.0f,  // left-right
};

float pyramid_colors[] =
{
    // triangle of front side
    1.0f, 0.0f, 0.0f, // R (top)
    0.0f, 1.0f, 0.0f, // G
    0.0f, 0.0f, 1.0f, // B
    
    // triangle of right side
    1.0f, 0.0f, 0.0f, // R (top)
    0.0f, 0.0f, 1.0f, // B
    0.0f, 1.0f, 0.0f, // G
    
    // triangle of back side
    1.0f, 0.0f, 0.0f, // R (top)
    0.0f, 1.0f, 0.0f, // G
    0.0f, 0.0f, 1.0f, // B
    
    // triangle of left side
    1.0f, 0.0f, 0.0f, // R (top)
    0.0f, 0.0f, 1.0f, // B
    0.0f, 1.0f, 0.0f, // G
};

float pyramid_normals[] =
{
    // front
    0.0f, 0.447214f, 0.894427f,
    0.0f, 0.447214f, 0.894427f,
    0.0f, 0.447214f, 0.894427f,
    
    // right
    0.894427f, 0.447214f, 0.0f,
    0.894427f, 0.447214f, 0.0f,
    0.894427f, 0.447214f, 0.0f,
    
    // back
    0.0f, 0.447214f, -0.894427f,
    0.0f, 0.447214f, -0.894427f,
    0.0f, 0.447214f, -0.894427f,
    
    // left
    -0.894427f, 0.447214f, 0.0f,
    -0.894427f, 0.447214f, 0.0f,
    -0.894427f, 0.447214f, 0.0f
};

float pyramid_texcoords[] =
{
    0.5, 1.0, // front-top
    0.0, 0.0, // front-left
    1.0, 0.0, // front-right
    
    0.5, 1.0, // right-top
    1.0, 0.0, // right-left
    0.0, 0.0, // right-right
    
    0.5, 1.0, // back-top
    1.0, 0.0, // back-left
    0.0, 0.0, // back-right
    
    0.5, 1.0, // left-top
    0.0, 0.0, // left-left
    1.0, 0.0, // left-right
};

float cube_vertices[] =
{
    // SIDE 1 ( TOP )
    // triangle 1
    -1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f,
    // triangle 2
    -1.0f, +1.0f, -1.0f,
    +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f,
    
    // SIDE 2 ( BOTTOM )
    // triangle 1
    +1.0f, -1.0f, -1.0f,
    +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f,
    // triangle 2
    -1.0f, -1.0f, -1.0f,
    +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f,
    
    // SIDE 3 ( FRONT )
    // triangle 1
    -1.0f, +1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    // triangle 2
    -1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    +1.0f, -1.0f, -1.0f,
    
    // SIDE 4 ( BACK )
    // triangle 1
    +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f,
    // triangle 2
    -1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f,
    
    // SIDE 5 ( LEFT )
    // triangle 1
    -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f,
    -1.0f, -1.0f, +1.0f,
    // triangle 2
    -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    
    // SIDE 6 ( RIGHT )
    // triangle 1
    +1.0f, -1.0f, -1.0f,
    +1.0f, +1.0f, -1.0f,
    +1.0f, -1.0f, +1.0f,
    // triangle 2
    +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f,
    +1.0f, +1.0f, +1.0f,
};

float cube_colors[] =
{
    // SIDE 1 ( TOP ) : RED
    // triangle 1 of side 1
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    // triangle 2 of side 1
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    
    // SIDE 2 ( BOTTOM ) : GREEN
    // triangle 1 of side 2
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    // triangle 2 of side 2
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    
    // SIDE 3 ( FRONT ) : BLUE
    // triangle 1 of side 3
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    // triangle 2 of side 3
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    
    // SIDE 4 ( BACK ) : CYAN
    // triangle 1 of side 4
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    // triangle 2 of side 4
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    
    // SIDE 5 ( LEFT ) : MAGENTA
    // triangle 1 of side 5
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    // triangle 2 of side 5
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    
    // SIDE 6 ( RIGHT ) : YELLOW
    // triangle 1 of side 6
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    // triangle 2 of side 6
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
};

float cube_normals[] =
{
    // SIDE 1 ( TOP )
    // triangle 1
    0.0f, +1.0f, 0.0f,
    0.0f, +1.0f, 0.0f,
    0.0f, +1.0f, 0.0f,
    // triangle 2
    0.0f, +1.0f, 0.0f,
    0.0f, +1.0f, 0.0f,
    0.0f, +1.0f, 0.0f,
    
    // SIDE 2 ( BOTTOM )
    // triangle 1
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    // triangle 2
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    
    // SIDE 3 ( FRONT )
    // triangle 1
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    // triangle 2
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    
    // SIDE 4 ( BACK )
    // triangle 1
    0.0f, 0.0f, +1.0f,
    0.0f, 0.0f, +1.0f,
    0.0f, 0.0f, +1.0f,
    // triangle 2
    0.0f, 0.0f, +1.0f,
    0.0f, 0.0f, +1.0f,
    0.0f, 0.0f, +1.0f,
    
    // SIDE 5 ( LEFT )
    // triangle 1
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    // triangle 2
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    
    // SIDE 6 ( RIGHT )
    // triangle 1
    +1.0f, 0.0f, 0.0f,
    +1.0f, 0.0f, 0.0f,
    +1.0f, 0.0f, 0.0f,
    // triangle 2
    +1.0f, 0.0f, 0.0f,
    +1.0f, 0.0f, 0.0f,
    +1.0f, 0.0f, 0.0f,
    
};

float cube_texcoords[] =
{
    // SIDE 1 ( TOP )
    // triangle 1
    +0.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +0.0f,
    // triangle 2
    +1.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +1.0f,
    
    // SIDE 2 ( BOTTOM )
    // triangle 1
    +0.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +0.0f,
    // triangle 2
    +1.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +1.0f,
    
    // SIDE 3 ( FRONT )
    // triangle 1
    +0.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +0.0f,
    // triangle 2
    +1.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +1.0f,
    
    // SIDE 4 ( BACK )
    // triangle 1
    +0.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +0.0f,
    // triangle 2
    +1.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +1.0f,
    
    // SIDE 5 ( LEFT )
    // triangle 1
    +0.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +0.0f,
    // triangle 2
    +1.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +1.0f,
    
    // SIDE 6 ( RIGHT )
    // triangle 1
    +0.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +0.0f,
    // triangle 2
    +1.0f, +0.0f,
    +0.0f, +1.0f,
    +1.0f, +1.0f,
};
