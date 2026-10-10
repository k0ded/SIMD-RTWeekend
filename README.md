# RayTracing in One Weekend
Final test includes a final render that renders an image that is 1200x675 500spp 50 bounces.

# Statistics
Benchmarks on my PC.
- CPU: Intel i5-11400F (6 Cores 12 threads)
- RAM: CORSAIR VENGANCE LPX 2x16GB DDR4 C16 @ 2133MHz

| SIMD TYPE | THREADS | EXECUTION TIME | TOTAL RAYS CAST | 
| --------- | ------- | -------------- | --------------- |
| SSE4      | 12      | 54.886s        | ~1.17b          |
| SSE4      | 6       | 75.948s        | ~1.17b          |
| AVX       | 12      | 119.954s       | ~1.28b          |
| AVX       | 6       | 212.086s       | ~1.28b          |
| AVX512    | 12      | 87.397s        | ~1.46b          |
| AVX512    | 6       | 144.563s       | ~1.46b          |

Surprisingly the time goes up with more registers but this makes more sense when looking at the code.
My implementation has the same issue as when you do raytracing in Pixel Shaders. 
When rays start to bounce they diverge and a lot of work just gets discarded. 
I will try a separate method where only one ray is evaluated by any given thread but multiple hittable items get evaluated at once (i.e 4-8-16 spheres getting evaluated at once)
Stay tuned :P
