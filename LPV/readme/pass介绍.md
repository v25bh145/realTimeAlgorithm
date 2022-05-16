## GetShadowSamplePass

### input

- 需要渲染的模型顶点数据，模型的材质和模型变换矩阵。

- 点光源六个面分别的转换矩阵lightShadowMatrices[6]，值为projection*view。

- 点光源位置、亮度以及其能渲染的最远平面。

### output

- 以点光源为中心的万向贴图，包括点光源照到的点的世界空间坐标worldPos以及辐射通量flux。

## LightInjectionPass

### input

- 立方体的八个顶点位置。
- 点光源六个面分别的转换矩阵lightShadowMatrices[6]，值为projection*view。
- GetShadowSamplePass中渲染的两张贴图。
- 一个格子的大小，由格子数量计算而来。
- 模型包围盒的最小点，由CPU端计算得出。

### output

- 六张球谐系数贴图。

## LightPropogationPass

### input

- 六张球谐系数贴图。
- 一个格子的大小，由格子数量计算而来。
- 模型包围盒的最小点，由CPU端计算得出。
- 格子数量。

### output

- 经过光照传播后的六张球谐系数贴图。

### GBufferPass

### input

- 需要渲染的模型顶点数据，模型的材质和模型变换矩阵。
- 照相机的视角变换以及投影变换矩阵。

### output

- 以照相机为中心的2D贴图，包括照相机可见的顶点的世界空间坐标、漫反射值以及法线值。

## LPVOutputPass

### input

- 点光源位置、亮度以及其能渲染的最远平面。

- 以照相机为中心的2D贴图，包括照相机可见的顶点的世界空间坐标、漫反射值以及法线值。
- GetShadowSamplePass中渲染的万向顶点位置贴图，用于计算阴影。
- GBufferPass中渲染的3张以照相机为中心的2D贴图。
- 一个格子的大小，由格子数量计算而来。
- 模型包围盒的最小点，由CPU端计算得出。
- 格子数量。
- 照相机的位置。

### output

- 直接光照+间接光照。





