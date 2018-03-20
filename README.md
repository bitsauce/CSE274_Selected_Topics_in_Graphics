# Project in sampling and reconstruction of visual appearance
# Axis-Aligned Filtering for Interactive Sampled Soft Shadows
# http://graphics.berkeley.edu/papers/UdayMehta-AAF-2012-12/
## by Asbjoern Lystrup and Marcus Loo Vergara

This project shows our implementation soft shadows based on the paper [*Axis-Aligned Filtering for Interactive Sampled Soft Shadows*](http://graphics.berkeley.edu/papers/UdayMehta-AAF-2012-12/) by Soham Uday Mehta, Brandon Wang, and Ravi Ramamoorthi. The soft shadows are based on Monte Carlo sampling, and is hence physically accurate. It uses planar area light sources. The method starts off by casting a number of rays per pixel to obtain values defining the occlusion. These values are then used to find filter widths for blurring the noise, and an additional sample count to further enhance accuracy in the most complex areas of the shadows. Then the filtering is applied. The filtering is axis-aligned and done in image-space, providing great performance and making interaction possible.

We started out by downloading and learning NVIDIA's OptiX, which is a real-time raytracing framework used by the paper. We looked at a couple of tutorials to understand the interaction between rays and geometry, and started to work on our implementation. 

The paper's method derives from recent work on frequency analysis and sheared filtering for offline soft shadows. The paper develops a theoretical analysis for axis-aligned filtering. After setting up the foundation of the implementation, we spent some time studying fourier spectrums to get a better idea of the paper's theory.

We continued working on our implementation and added occlusion calculation and debugging functionality. For the occlusion calculation, nine rays per pixel are sent toward random points on the light source to obtain distances between the light source and the closest and furthest occluder. We approximate the distance between the light source and the pixels by using the center of the light source. In the same pass, we calculate the filter widths using the equations from the paper and store them in a float buffer. The debugging functionality we implemented let us use the arrow keys to move back and forth to look at different buffers, visualized by normalizing the buffer to make the greatest value correspond to white, and the lowest to black. It also let us see the minimum, average, and maximum value in text form, as well as the framerate.

We then implemented the axis-aligned filtering. We separate the filtering into two passes based on separable convolution; a pass that blurs the shadows horizontally, followed by a pass that blurs the first pass' result vertically. The computed filter widths correspond to standard deviations in a gaussian distribution used for the blurring. To avoid sampling from other objects, we created an object ID buffer where each object's pixels gets its own unique value.

We went back to the pass for the filter width calculation, and implemented adaptive sampling. The adaptive sampling is used to improve both the diffuse accuracy and the filter widths, and is based on the equations from the paper, which uses the occlusion distances. We use an upper limit of 100 samples per pixel. To compare our results with ground truth, we extended our debugging tools to generate three images upon a button press; a filtered result image, a ground truth image, and a disparity map.

As of this time, we had quite decent results, and we held a presentation of our project in class. Our filtering used gaussian offsets corresponding to image-space pixel positions. We had started writing code for world-space based gaussian offsets, but it wasn't complete, so we continued to work on this. The approach we used was to compute and store the 2D light-parallel position of each pixel in the same pass as the filter width and adaptive sampling calculations. To compute the positions, we created a change-of-basis matrix using the light's vectors, with the normal vector in the third column, then multiplied it by each pixel's world position and discarded the z-component. In the filter passes, we used the distance between the center pixel and the neighboring pixels' light-parallel positions as gaussian offsets, in accordance to the theory of the paper. Surprisingly enough, the results we obtained were nearly identical to the results from the original image-space offsets, also when comparing the disparity maps. So you could skip this step and use the image-space offsets to get a small performance boost, but this might also cause you to run into artefacts under certain scenarios as it is not as physically accurate as the world-space based offsets.

<p align="center">
  <img src="optixSoftShadows/screenshots/flower_diffuse.png">
</p>

<p align="center">
  <img src="optixSoftShadows/screenshots/flower_filtered.png">
</p>

<p align="center">
  <img src="optixSoftShadows/screenshots/flower_ground_truth.png">
</p>

<p align="center">
  <img src="optixSoftShadows/screenshots/flower_difference.png">
</p>

In addition to this, we added some new models to better test self-shadowing and complex shadows. We extended our filter's object ID check to also compare normals; each sample is required to be within a specified angle of the center sample to be included in the blurred result. We also added a filter that averages the occlusion distances in a 10x10 pixel area for completely unoccluded pixels to reduce noise for these pixels. Like our main filter, this filter also consists of two passes and is based on separable convolution. Besides this, we changed the grayscale debug visualization to a heatmap.

From this we have learned that physically accurate soft shadows can be sampled very efficiently at interactive framerates. The paper was published in 2012, and we can see clear improvements in framerate after having tested our implementation on newer graphics cards. The paper used an NVIDIA GTX 570 for its test results. We used one of today's corresponding models, the NVIDIA GTX 970. From hardware evolution, as well as from research progress in the area, we can see that we are getting closer and closer to raytracing being a viable option for real-time applications.
