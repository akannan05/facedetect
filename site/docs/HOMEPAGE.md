---
layout: page
title: "introduction"
permalink: /home
order: 1
---

# a quick introduction

Recently, I've been getting more and more interested in working with cameras at a lower level. Up till now, when wanting to access a video feed or frame from a camera device attached to my computer I would use OpenCV's `cv2.VideoCapture(id)`. 

This is enough for a majority of use cases at this scale, and honestly, this project *is* kind of reinventing the wheel. However, I want to treat this as a learning experience, rather than a project I do for the sole purpose of putting it on my resume. I also want to refresh on my C, and improve my ability of trudging through documentation.

My hardware is pretty limited. I'll be using an endoscopy camera (that I actually have for a separate vision project), as my video capture device, and all the processing will be done on my laptop for now. In the future, I do plan on changing this to a low power processing system like the ESP32-S3 or STM32H7, just so I can test in on something where its actually useful. 

Self-imposed rules for my project:
1. No OpenCV. I'll be writing my own image processing pipeline from scratch in C/C++
2. Stay away from AI tools as much as possible. I'm gonna try to force myself to read as much documentation as possible, while I may use AI to point me in the right direction.
3. Document as much as possible. I'm hoping this will help me retain the information better for future reference.

The main steps of this project are as follows:
1. Write a hardware abstraction layer (HAL) using V4L2, enabling raw (YUYV) frame and feed capture from camera.
2. Write a pipeline for color space conversion (YUYV -> RGB), and other basic preprocessing steps.
3. Expand the custom pipeline to enable Haar Cascade Classifier (gaussian blur, integral image, etc.)
4. Experiment with different methods of similarity scoring or some metric to recognize when a face has been recognized
5. Port the entire pipeline and HAL for usage to unlock a mobile device (e.g: Android via Android Emulator)
6. Go back through and use NEON intrinsics and other CPU and ARM optimization techniques.
7. Benchmark my pipelines performance in comparison to an OpenCV pipeline.

