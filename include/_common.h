#ifndef _COMMON_H
#define _COMMON_H

#include<iostream>
#include<windows.h>
#include<string>

#include<gl/gl.h>
#include<GL/glut.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<chrono>

#include<fstream>
#include<vector>
#include <sstream>
#include <limits>

#include"_Time.h"

#define PI 3.14159

using namespace std;

typedef struct{
          float x;
          float y;
          } vec2;

typedef struct{
          float x;
          float y;
          float z;
          } vec3;

typedef struct{
          float r;
          float g;
          float b;
          float a;
          } col4;

struct Vector3{
    float x,y,z;

    Vector3(){x=y=z=0;}
    Vector3(float xVal, float yVal, float zVal) : x(xVal), y(yVal), z(zVal) {}

    Vector3 operator+(Vector3& other){
        return Vector3(x+other.x, y+other.y, z+other.z);
    }

    Vector3 operator-(Vector3& other){
        return Vector3(x-other.x, y-other.y, z-other.z);
    }

    Vector3 operator*(float scalar){
        return Vector3(x*scalar, y*scalar, z*scalar);
    }

    Vector3 operator/(float scalar){
        return Vector3(x/scalar, y/scalar, z/scalar);
    }

};

struct Vector2{
    float x,y;

    Vector2(){x=y=0;}
    Vector2(float xVal, float yVal) : x(xVal), y(yVal) {}

    Vector2 operator+(Vector2& other){
        return Vector2(x+other.x, y+other.y);
    }
};

#endif // _COMMON_H
