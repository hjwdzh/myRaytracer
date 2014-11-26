myRaytracer
===========

GLSL Raytracer

Use make to compile the project.

Run:

./render (-bvh) (-path 1) -input raytracing.scene (-output result.bmp)

-bvh: bound volume hierarchy acceleration
-path number: replace the simple tracer with path tracer. number specifies maximum number of diffuse material a ray hits. Suggesting number is 1.
raytracing.scene is a scene file used to specify the scene.
-output path: output the image (only support bitmap).
