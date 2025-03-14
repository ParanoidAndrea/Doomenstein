#pragma once
#include "Engine/Math/MathUtils.hpp"

float EaseInQuadratic(float t);  //SmoothStart2
float EaseInCubic(float t);	     //SmoothStart3
float EaseInQuartic(float t); 	 //SmoothStart4
float EaseInQuintic(float t);	 //SmoothStart5
float EaseIn6thOrder(float t);	 //SmoothStart6

float EaseOutQuadratic(float t); //SmoothStop2
float EaseOutCubic(float t);	 //SmoothStop3
float EaseOutQuartic(float t);   //SmoothStop4
float EaseOutQuintic(float t);	 //SmoothStop5
float EaseOut6thOrder(float t);  //SmoothStop6

float SmoothStart2 (float t);
float SmoothStart3 (float t);
float SmoothStart4 (float t);
float SmoothStart5 (float t);
float SmoothStart6 (float t);
float SmoothStop2  (float t);
float SmoothStop3  (float t);
float SmoothStop4  (float t);
float SmoothStop5  (float t);
float SmoothStop6(float t);

float SmoothStep3(float t);
float SmoothStep5(float t);
float Hesitate3(float t);
float Hesitate5(float t);

float MyCustomFunkyEasing(float t);