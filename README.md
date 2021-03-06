# Implementation of Axis-Aligned Filtering for Interactive Sampled Soft Shadows
## by Asbjoern Lystrup and Marcus Loo Vergara

## Background

This page details our implementation of soft shadows based on the paper [*Axis-Aligned Filtering for Interactive Sampled Soft Shadows*](http://graphics.berkeley.edu/papers/UdayMehta-AAF-2012-12/) by Soham Uday Mehta, Brandon Wang, and Ravi Ramamoorthi. The paper describes a method of rendering interactive (15-30 FPS) soft shadows by first sampling stochastic rays (Monte Carlo ray tracing,) and then, afterward, analyzing the Fourier spectrum of the occlusion function of the sampled rays to determine how many additional rays to sample per-pixel and how to optimally blur shadows with a spatially varying screen-space Gaussian filter. Here's a quick summary of the method:

<p align="center">
  <img src="figures/Occlusion_Spectrum_Figure.png">
</p>

Given a planar area light source, we sample the distances from the geometry to the light source, and the maximum and minimum distances from the occluder to the light source are shown in the figure above as d1, d2 respectively. This gives us the 2-dimensional occlusion function, (b), which we can calculate the Fourier transform of, as shown in (c). Figure (c) shows that the occlusion function (shadows) can confine within a double-wedge, which can be described in terms of _d1_ and _d2_. By only considering samples within this double-wedge, we can substantially reduce the number of samples we need for accurate reconstructions, by blurring and sampling more in high-frequency regions than in low-frequency regions  - this is the main idea of the paper.

## Implementation

The implementation can be found [here](https://github.com/bitsauce/Axis-Aligned-Filtering-Soft-Shadows). Our implementation uses NVIDIA's [OptiX](https://developer.nvidia.com/optix) to do real-time raytracing.

### Sample Distances

In our implementation, we start by casting one ray per pixel to find the primary geometry hit. From this hit location, we now cast 9 rays per pixel towards 9 random points the area light source. This gives us values for _d2_max_ and _d2_min_ as illustrated below.

<p align="center">
  <img src="figures/d1_d2_figure.png">
</p>

(a) shows the values of _d1_ for every pixel, and is just the Euclidean distance from the primary geometry hit to the center of the light source. (b) shows the smallest distance from the closest occluder to the light source out of the 9 _d2_ values that were sampled. Conversely, (c) shows the maximum _d2_ that was sampled out of the 9. We observe that _d2_min_ is biggest near the corners of the boxes. This makes sense since, in corners, most rays hit the box pretty far from the light source. In areas away from the box, _d2_min_ and _d2_max_ vary a lot more than in corners. We will exploit this to apply a spatially varying Gaussian blur later.

<p align="center">
  <img src="optixSoftShadows/screenshots/boxes_diffuse.png">
</p>

While we sample the 9 values for _d2_, we also sample the intensity of the color, giving us the noisy output we will blur later, as seen above.

### Adaptive Sampling

So, now we have _d1_, _d2_min_ and _d2_max_. At this point, we calculate the number of additional samples using the formula given in the paper:

<p align="center">
  <img src="figures/adaptive_sampling_formula.png">
</p>

This tells us how many extra samples to do. The idea is that we need more information in high-frequency areas, so we do additional samples in those regions. In our implementation, we decided to limit the maximum number of samples to 100 for performance reasons. We also update the color, _d1_, _d2_min_, and _d2_max_ as we're doing these additional samples. The figure below shows a visualization of the adaptive sampling.

<p align="center">
  <img src="optixSoftShadows/screenshots/boxes_num_samples.png">
</p>

We can see here that it will sample more in more complex regions, improving the quality of shadows.

After obtaining our values of _d2_, we also do 2D a gaussian average over _d1_ and _d2_ in a 5-pixel radius. This is to remove some noise introduced by pixels completely unoccluded pixels.

### Standard Deviation Calculation

Now we can finally calculate the standard deviation of the gaussian blur which is to be applied over the screen. This value is given by the following formula:

<p align="center">
  <img src="figures/beta_formula.PNG">
</p>

Below is a visualization of the betas we obtained:

<p align="center">
  <img src="optixSoftShadows/screenshots/boxes_beta.png">
</p>

Here we can clearly see that this method will blur more on the outer edges of the shadows, giving the shadows a nice looking penumbra after blurring. 

### Image-Space Blur

<p align="center">
  <img src="figures/gauss_formula.png">
</p>

We apply the betas we calculated from the previous step to do a spatially-varying gaussian blur in image-space, given by the formula above. Beta correspond to the standard deviations of the gaussian blur at a given pixel. We also need to calculate the projected distance _x-x'_. This value represents the distance between the pixels, parallel to the light plane. To compute this, we created a change-of-basis matrix using the light's vectors, with the normal vector in the third column, then multiplied it by each pixel's world position and discarded the z-component.

Finally, we separate the gaussian blur a horizontal pass and a vertical pass. To avoid blurring across objects, compare the object ID and surface normal of the pixel before applying the blur. This produces the final image:

<p align="center">
  <img src="optixSoftShadows/screenshots/boxes_filtered.png">
</p>

## Results

We are satisfied with the results we got. Our method is currently running at interactive frame-rates (~10-30 frames per second) on a GeForce GTX 970. Throughout the process, we found that there are some simplifications which could be made. For example, when we were implementing the offset in the gaussian for the image-space blur, we found that the results are nearly identical to the results from just using the image-space (pixel) offsets. As such, you could skip this step and use the image-space offsets to get a small performance boost, but this might also cause you to run into artifacts under specific scenarios as it is not as physically accurate as the world-space based offsets.

In addition to this, we created a new scene to better test self-shadowing and complex shadows. Here are some screenshots from the scene:

<p align="center">
  <img src="optixSoftShadows/screenshots/grid_beta.png">
</p>

<p align="center">
  <img src="optixSoftShadows/screenshots/grid_num_samples.png">
</p>

<p align="center">
  <img src="figures/grid_figure.png">
</p>

<p align="center">
  <img src="figures/flower_figure.png">
</p>

Above is a sequence of images showing soft shadows for a grid and a plant model. (a) shows the unfiltered version of the soft shadows. (c) shows the filtered result. (d) is the ground truth, generated by sampling 4000 rays per pixel. (b) shows the difference between the filtered result and the ground truth scaled by a factor of 20. The difference is small, but the filtered result looks slightly more smudgy in the shadows, due to noise from the initial, unfiltered sampling. Also, if you look at, for example, the stem of the flower, you can see that the filtered result has blurred the sharp, black edge from the ground truth image. The brightest regions in the difference map are a result of blurring artifacts like this one. The reason for this blurring occurring is that the filter width is not exactly zero in those regions, causing some areas with very high occlusion frequencies to be blurred a little.

## Conclusion

From this, we have learned that physically accurate soft shadows can be sampled very efficiently at interactive framerates. The paper was published in 2012, and we can already see clear improvements in framerate due to advancements in modern graphics cards performance. With recent publications in real-time raytracing, such as NVIDIA's [*Spatiotemporal Variance-Guided Filtering*](http://research.nvidia.com/publication/2017-07_Spatiotemporal-Variance-Guided-Filtering%3A), we can see that real-time raytracing is likely going to be a viable option for real-time graphics in the near future.
