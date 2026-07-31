#ifndef VOLTAGE_TO_LINE_H
#define VOLTAGE_TO_LINE_H


typedef enum
{
    LINE_NONE,
    LINE_FRONT,  // front 2 sensors detect white 
    LINE_BACK,    // back 2 sensor detect white
    LINE_LEFT,     // left side sensor detect white 
    LINE_RIGHT,     // right side sensor detect white
    LINE_FRONT_LEFT,   // only front left sensor detects
    LINE_FRONT_RIGHT,  // only front right sensor detects
    LINE_BACK_LEFT,    // only back left 
    LINE_BACK_RIGHT,   // only back right 
    LINE_DIAGONAL_LEFT,  // diagonal left (front left and back right)
    LINE_DIAGONAL_RIGHT  // diagonal right (front right and back left)
}line_e;


void line_init(void);
 line_e line_get(void);


#endif