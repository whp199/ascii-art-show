#include "art.h"
#include <math.h>
#include <stdlib.h> // For abs()

typedef struct { float x, y, z; } Point3D;
static Point3D points[8];
static float angle_x = 0, angle_y = 0;

void cube_init(int width, int height) {
    (void)width; (void)height;
    int i = 0;
    for (int x = -1; x <= 1; x += 2)
        for (int y = -1; y <= 1; y += 2)
            for (int z = -1; z <= 1; z += 2)
                points[i++] = (Point3D){(float)x, (float)y, (float)z};
}

void cube_update(double progress, double time_elapsed) {
    (void)progress;
    angle_x = time_elapsed * 0.5f;
    angle_y = time_elapsed * 0.3f;
}

void cube_draw(ScreenBuffer *buffer) {
    Point3D transformed[8];
    for (int i = 0; i < 8; i++) {
        // Rotate Y
        float rz = points[i].z * cosf(angle_y) - points[i].x * sinf(angle_y);
        float rx = points[i].z * sinf(angle_y) + points[i].x * cosf(angle_y);
        float ry = points[i].y;
        // Rotate X
        float rry = ry * cosf(angle_x) - rz * sinf(angle_x);
        float rrz = ry * sinf(angle_x) + rz * cosf(angle_x);
        transformed[i] = (Point3D){rx, rry, rrz + 4.0f}; // Z offset
    }

    int projected[8][2];
    for (int i = 0; i < 8; i++) {
        float z = transformed[i].z;
        projected[i][0] = (int)(transformed[i].x / z * buffer->width * 0.5 + buffer->width / 2);
        projected[i][1] = (int)(transformed[i].y / z * buffer->height * 0.5 + buffer->height / 2);
    }

    Color c = {200, 200, 255};
    for (int i = 0; i < 4; i++) {
        // Draw lines connecting the front and back faces of the cube
        buffer_draw_line(projected[i][0], projected[i][1], projected[i+4][0], projected[i+4][1], '#', c);
        // Draw the front face
        buffer_draw_line(projected[i][0], projected[i][1], projected[(i&2)?i-2:i+2][0], projected[(i&2)?i-2:i+2][1], '#', c);
        buffer_draw_line(projected[i][0], projected[i][1], projected[(i&1)?i-1:i+1][0], projected[(i&1)?i-1:i+1][1], '#', c);
        // Draw the back face
        buffer_draw_line(projected[i+4][0], projected[i+4][1], projected[(i&2)?i+2:i+6][0], projected[(i&2)?i+2:i+6][1], '#', c);
        buffer_draw_line(projected[i+4][0], projected[i+4][1], projected[(i&1)?i+3:i+5][0], projected[(i&1)?i+3:i+5][1], '#', c);
    }
}

ArtModule get_cube_module() {
    return (ArtModule){
        .name = "cube",
        .description = "A rotating 3D wireframe cube",
        .init = cube_init,
        .update = cube_update,
        .draw = cube_draw,
        .destroy = NULL,
    };
}
